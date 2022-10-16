#pragma once
#include <vector>

class Neuron;
typedef std::vector<Neuron> Layer;

class Network {
public:
    Network(const std::vector<int> &topology); 
    ~Network();

    void feedForward(const std::vector<double> &inputData);
    void backPropagation(const std::vector<double> &targetData);
    void result(std::vector<double> &resultData) const;

    double avgError() {return recAvgError;}

private:
    double netError;
    double recAvgError;
    double recAvgSmoothing;

    std::vector<Layer> netLayers;  // 2D vector
};