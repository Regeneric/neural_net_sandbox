#pragma once
#include <vector>

#define SMOOTHING 100.0

class Neuron;
typedef std::vector<Neuron> Layer;

class Network {
public:
    // Network.cpp
    Network(const std::vector<int> &topology);

    void feedForward(const std::vector<double> &inputData);
    void backPropagation(const std::vector<double> &targetData, std::vector<KNOWN_WEIGHTS> &trainedWeights, int &iter, bool useTrained = false);
    void result(std::vector<double> &resultData);
    // ~Network.cpp

    double avgError() {return recAvgError;}

private:
    double netError = 0.0;
    double recAvgError = 0.0;
    static double recAvgSmoothing;

    std::vector<Layer> netLayers;  // 2D vector
};