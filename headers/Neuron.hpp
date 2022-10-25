#pragma once
#include <vector>
#include <cmath>

#include "Connection.hpp"

#define ETA         0.15
#define ALPHA       0.50

struct KNOWN_WEIGHTS {
    int id;
    double weight;
};

class Neuron;
typedef std::vector<Neuron> Layer;

class Neuron {
public:
    // Neuron.cpp
    Neuron(int outputs, int nID);

    void inputWeights(Layer &prevLayer, std::vector<KNOWN_WEIGHTS> &trainedWeights, int &iter, bool useTrained = false);
    std::vector<Connection> inputWeights();

    void feedForward(const Layer &prevLayer);

    void outputGradient(double targetData);
    void hiddenGradient(const Layer &nextLayer);
    // ~Neuron.cpp
    

    int id() {return _nID;}
    double weight() const {return _weights[_nID].weight();}
    void weight(double weight) {_weights[_nID].weight(weight);}

    void output(double outputs) {_output = outputs;}
    double output() const {return _output;}

private:
    int _nID;
    
    double _output;
    double _gradient;

    static double eta;
    static double alpha;

    std::vector<Connection> _weights;

    // Hyperbolic tanget function - https://en.wikipedia.org/wiki/Hyperbolic_functions#Hyperbolic_tangent
    // Scalling output between -1.0 and 1.0
    static double transfer(double x);
    static double transferDeriv(double x);   // Very close approximation
    double sumDOW(const Layer &nextLayer) const;
};