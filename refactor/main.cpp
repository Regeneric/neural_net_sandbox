#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <cstdlib>
#include <cassert>
#include <cmath>


struct Connection {
    double weight;
    double deltaWeight;
};


// --------------------------------------------------------- //
class Neuron;
typedef std::vector<Neuron> Layer;

class Neuron {
public:
    Neuron(int outputs, int id);

    void outputVal(double val) {output = val;}
    double outputVal() const {return output;}

    void feedForward(const Layer &prevLayer);
    void outputGradients(double targetVal);
    void hiddenGradients(const Layer &nextLayer);

    void inputWeight(Layer &prevLayer);

private:
    int nID;

    double output;
    double gradient;

    static double eta;
    static double alpha;

    std::vector<Connection> weights;


    double sumDOW(const Layer &nextLayer) const;

    static double transferFunc(double x);
    static double transferFuncDeriv(double x);

    static double randomWeight() {return rand() / double(RAND_MAX);}
};


Neuron::Neuron(int outputs, int id) {
    for(int c = 0; c < outputs; ++c) {
        weights.push_back(Connection());
        weights.back().weight = randomWeight();
    } nID = id;
}

double Neuron::eta = 0.15;
double Neuron::alpha = 0.5;

void Neuron::inputWeight(Layer &prevLayer) {
    for(int n = 0; n < prevLayer.size(); ++n)  {
        Neuron &neuron = prevLayer[n];

        double oldDeltaWeight = neuron.weights[nID].deltaWeight;
        double newDeltaWeight = eta * neuron.outputVal() * gradient + alpha * oldDeltaWeight;

        neuron.weights[nID].deltaWeight = newDeltaWeight;
        neuron.weights[nID].weight += newDeltaWeight;
    } return ;
}

double Neuron::sumDOW(const Layer &nextLayer) const {
    double sum = 0.0;
    for(int n = 0; n , nextLayer.size()-1; ++n) 
        sum += weights[n].weight * nextLayer[n].gradient;

    return sum;
}

void Neuron::hiddenGradients(const Layer &nextLayer) {
    double dow = sumDOW(nextLayer);
    gradient = dow * transferFuncDeriv(output);

    return;
}

void Neuron::outputGradients(double targetVal) {
    double delta = targetVal - output;
    gradient = delta * transferFuncDeriv(output);

    return;
}

double Neuron::transferFunc(double x) {return tanh(x);}
double Neuron::transferFuncDeriv(double x) {return 1.0 - x*x;}

void Neuron::feedForward(const Layer &prevLayer) {
    double sum = 0.0;
    for(int n = 0; n < prevLayer.size(); ++n) {
        sum += prevLayer[n].outputVal() *
               prevLayer[n].weights[nID].weight;
    } output = transferFunc(sum);

    return;
}
// --------------------------------------------------------- //


// --------------------------------------------------------- //
class Network {
public:
    Network(const std::vector<int> &topology);

    void feedForward(const std::vector<double> &inputVals);
    void backProp(const std::vector<double> &targetVals);
    void netResults(std::vector<double> &resultVals) const;

    double avgError() const {return recAvgError;}

private:
    std::vector<Layer> netLayers;

    double netError;
    double recAvgError;

    static double recAvgSmoothing;
};


Network::Network(const std::vector<int> &topology) {
    int numLayers = topology.size();
    std::cout << "topology.size(): " << numLayers << std::endl;
    for(int nl = 0; nl < numLayers; ++nl) {
        netLayers.push_back(Layer());
        int outputs = (nl == topology.size()-1) ? 0 : topology[nl+1];

        for(int id = 0; id <= topology[nl]; ++id) {
            netLayers.back().push_back(Neuron(outputs, id));
            std::cout << "Added " << id+1 << " neuron to layer " << nl+1 << std::endl;
        } 
        
        // Bias neuron
        netLayers.back().back().outputVal(1.0);
    }
}

double Network::recAvgSmoothing = 100.0;

void Network::netResults(std::vector<double> &resultVals) const {
    resultVals.clear();
    for(int n = 0; n < netLayers.back().size()-1; ++n) 
        resultVals.push_back(netLayers.back()[n].outputVal());

    return;
}

void Network::backProp(const std::vector<double> &targetVals) {
    Layer &outputLayer = netLayers.back();
    netError = 0.0;

    for(int n = 0; n < outputLayer.size()-1; ++n) {
        double delta = targetVals[n] - outputLayer[n].outputVal();
        netError += delta*delta;
    }

    netError /= outputLayer.size()-1;
    netError = sqrt(netError);


    recAvgError = (recAvgSmoothing * recAvgSmoothing + netError) / (recAvgSmoothing + 1.0);
    for(int n = 0; n < outputLayer.size()-1; ++n) outputLayer[n].outputGradients(targetVals[n]);


    for(int nl = netLayers.size()-2; nl > 0; --nl) {
        Layer &hiddenLayer = netLayers[nl];
        Layer &nextLayer = netLayers[nl+1];

        for(int n = 0; n < hiddenLayer.size(); ++n) hiddenLayer[n].hiddenGradients(nextLayer);
    }


    for(int nl = netLayers.size()-1; nl > 0; --nl) {
        Layer &currLayer = netLayers[nl];
        Layer &prevLayer = netLayers[nl-1];

        for(int n = 0; n < currLayer.size()-1; ++n) currLayer[n].inputWeight(prevLayer);
    }

    return;
}

void Network::feedForward(const std::vector<double> &inputVals) {
    assert(inputVals.size() == netLayers[0].size()-1);
    for(int i = 0; i < inputVals.size(); ++i)
        netLayers[0][i].outputVal(inputVals[i]);

    for(int nl = 1; nl < netLayers.size(); ++nl) {
        Layer &prevLayer = netLayers[nl-1];
        for(int n = 0; n < netLayers[nl].size()-1; ++n) {
            netLayers[nl][n].feedForward(prevLayer);
        }
    }

    return;
}
// --------------------------------------------------------- //


auto main() -> int {
    // Topology {X, Y, Z} - X inputs, Y neurons in hidden layer, Z outputs
    std::vector<int> topology{2, 4, 1};
    Network net(topology);

    std::vector<double> inputData;
    std::vector<double> targetData;
    std::vector<double> resultData;
    std::vector<int> labels;


    std::ifstream file;
    std::string label;  std::string line;
    
    int iteration = 0;
    while(iteration++ != 5) {
        file.open("or.txt");
        std::cout << "\nStarting iteration " << iteration << std::endl;

        while(!file.eof()) {
            getline(file, line);
            std::stringstream ss(line);
            ss >> label;

            if(label.compare("desc:") == 0) {
                labels.clear();
                int num = 0;
                while(ss >> num) labels.push_back(num);
            }

            if(label.compare("in:") == 0) {
                inputData.clear();
                double val = 0.0f;
                while(ss >> val) inputData.push_back(val);
            }

            if(label.compare("out:") == 0) {
                targetData.clear();
                double val = 0.0f;
                while(ss >> val) targetData.push_back(val);
            }

            if(inputData.size() <= 0 || targetData.size() <= 0) continue;


            std::cout << std::endl;
            std::cout << "Number: ";
            for(const auto &l : labels) std::cout << l << std::endl;

            std::cout << "Input: ";
            for(const auto &id : inputData) std::cout << std::fixed << std::setprecision(1) << id << " ";

            net.feedForward(inputData);
            net.netResults(resultData);

            std::cout << std::endl;
            for(const auto &rd : resultData) std::cout << "Output: " << std::fixed << std::setprecision(4) << rd << std::endl;
            for(const auto &td : targetData) std::cout << "Target: " << std::fixed << std::setprecision(4) << td << std::endl;

            net.backProp(targetData);
            std::cout << "Avg Error: " << net.avgError() << std::endl << std::endl;
        } 

        std::cout << "End of iteration " << iteration << std::endl;
        
        file.close();
        file.clear();
        file.seekg(0, std::ios::beg);
    }
    return 0;
}