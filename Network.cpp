#include <iostream>
#include <cassert>

#include "headers/Neuron.hpp"
#include "headers/Network.hpp"

Network::Network(const std::vector<int> &topology) {
    for(int t = 0; t < topology.size(); ++t) {      // I need integer iterator for nested loop, hence there's no ranged-for or vector iterators
        netLayers.push_back(Layer());
        int outputs = (t == topology.size()-1) ? 0 : topology[t+1];     // If it's output layer, we don't need a neuron to have its own output
                                                                        // `t+1` because we want `Layer 1' neurons to have `Layer 2` neurons count outputs 

        for(int n = 0; n <= topology[t]; ++n) {                 // Adding one more neuron than declared - `bias neuron` - not needed in ouput layer
            netLayers.back().push_back(Neuron(outputs, n));     // Push neuron to latest added layer
            std::cout << "Added " << n+1 << "th neuron to layer " << t+1 << std::endl;
        }

        // Setup bias neuron
        netLayers.back().back().output(1.0);
    }
} Network::~Network(){}


void Network::feedForward(const std::vector<double> &inputData) {
    assert(inputData.size() == netLayers[0].size() - 1);                                // Make sure that number of input values == input layer neurons
    for(auto index = 0; const auto &id : inputData) netLayers[0][index++].output(id);   // Hold input value in input layer neuron  

    for(int nl = 1; nl < netLayers.size(); ++nl) {
        Layer &prevLayer = netLayers[nl-1];
        for(int n = 0; n < netLayers[nl].size()-1; ++n) {
            netLayers[nl][n].feedForward(prevLayer);
        }
    } return;
}

void Network::backPropagation(const std::vector<double> &targetData) {
    // Root Mean Sqaure Error (RMS) - https://en.wikipedia.org/wiki/Root-mean-square_deviation
    Layer &outputLayer = netLayers.back();
    netError = 0.0;

    for(int n = 0; n < outputLayer.size()-1; ++n) {
        double delta = targetData[n] - outputLayer[n].output();
        netError += delta * delta;
    }


    netError /= outputLayer.size() - 1;     // Get the average
    netError = sqrt(netError);              // RMS

    // Recent average measure - indicator how well the net is doing
    recAvgError = recAvgSmoothing + netError / (recAvgSmoothing + 1.0);

    for(int n = 0; n < outputLayer.size()-1; ++n) outputLayer[n].outputGradient(targetData[n]);

    for(int nl = netLayers.size()-2; nl > 0; --nl) {
        Layer &hiddenLayer = netLayers[nl];
        Layer &nextLayer = netLayers[nl+1];

        // for(int n = 0; n < hiddenLayer.size(); ++n) hiddenLayer[n].hiddenGradient(nextLayer);
        for(auto &hl : hiddenLayer) hl.hiddenGradient(nextLayer);
    }


    // Update connection weights
    for(int nl = netLayers.size() - 1; nl > 0; --nl) {  // Fromt outputs to first hidden layer
        Layer &currLayer = netLayers[nl];
        Layer &prevLayer = netLayers[nl-1];

        for(auto &l : currLayer) l.inputWeights(prevLayer);
    } return;
}

void Network::result(std::vector<double> &resultData) const {
    resultData.clear();

    // !!! C++20 !!!
    for(auto index = 0; const auto &nl : netLayers.back()) {
        if(++index == netLayers.size()-1) continue;
        resultData.push_back(nl.output());
    }

    for(const auto &rd : resultData) std::cout << rd << std::endl;
}