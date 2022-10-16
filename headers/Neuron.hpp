#pragma once
#include <vector>
#include <cmath>

#include "Connection.hpp"

class Neuron;
typedef std::vector<Neuron> Layer;

class Neuron {
public:
    Neuron(int outputs, int nID);
    ~Neuron();

    void feedForward(const Layer &prevLayer);
    void outputGradient(double targetData);
    void hiddenGradient(const Layer &nextLayer);
    void inputWeights(Layer &prevLayer);

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
    static double transfer(double x) {return tanh(x);}
    static double transferDeriv(double x) {return 1.0 - x * x;}   // Very close approximation

    double sumDOW(const Layer &nextLayer) const;
};