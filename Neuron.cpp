#include <vector>
#include <iostream>
#include "headers/Neuron.hpp"

double Neuron::transfer(double x) {return tanh(x);}
double Neuron::transferDeriv(double x) {return 1.0 - x*x;}

double Neuron::eta = ETA;       // Net learning speed  -  0.0 - 1.0
double Neuron::alpha = ALPHA;   // Multiplier of last weight  -  0.0 - n


Neuron::Neuron(int outputs, int nID){
    for(int o = 0; o < outputs; ++o) _weights.push_back(Connection());
    _nID = nID;
}


void Neuron::feedForward(const Layer &prevLayer) {
    double sum = 0.0;

    for(int n = 0; n < prevLayer.size(); ++n) {
        sum += prevLayer[n].output() * prevLayer[n]._weights[_nID].weight();
    }

    _output = Neuron::transfer(sum);    // Neuron:: not needed, added for my sanity
    return ;
}


void Neuron::outputGradient(double targetData) {
    double delta = targetData - _output;
    _gradient = delta * Neuron::transferDeriv(_output);

    return;
}

void Neuron::hiddenGradient(const Layer &nextLayer) {
    double dow = sumDOW(nextLayer);
    _gradient = dow * Neuron::transferDeriv(_output);

    return;
}

void Neuron::inputWeights(Layer &prevLayer, std::vector<KNOWN_WEIGHTS> &trainedWeights, int &iter, bool useTrained) {
    for(int n = 0; n < prevLayer.size(); ++n) {
        Neuron &neuron = prevLayer[n];

        if(!useTrained) {
            double oldWeight = neuron._weights[_nID].weightChange();
            double newWeight = eta * neuron.output() * _gradient + alpha * oldWeight;

            double buff = neuron._weights[_nID].weight();
            buff += newWeight;

            neuron._weights[_nID].weightChange(newWeight);
            neuron._weights[_nID].weight(buff);   

            KNOWN_WEIGHTS kw;
                kw.id = _nID;
                kw.weight = buff;
            trainedWeights.push_back(kw);
        
            if(n == prevLayer.size()-1) {iter += n; iter += 1;}
        } else {
            neuron._weights[trainedWeights[iter+n].id].weight(trainedWeights[iter+n].weight);
            if(n == prevLayer.size()-1) {iter += n; iter += 1;}
        }
    } return;
} std::vector<Connection> Neuron::inputWeights() {return _weights;}


double Neuron::sumDOW(const Layer &nextLayer) const {
    double sum = 0.0;

    for(int n = 0; n < nextLayer.size()-1; ++n) {
        sum += _weights[n].weight() * nextLayer[n]._gradient;
    } return sum;
}