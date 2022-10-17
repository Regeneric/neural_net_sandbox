#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>


#include <cstdlib>
#include <cassert>
#include <cmath>


class Neuron;
typedef std::vector<Neuron> Layer;


class Connection {
public:
    Connection() {_weight = randomizeWeight();}     // Assing random weight - to be trained


    void weight(double weight) {_weight = weight;}
    double weight() const {return _weight;}

    void weightChange(double weightChange) {_weightChange = weightChange;}
    double weightChange() const {return _weightChange;}

private:
    double _weight = 0;
    double _weightChange = 0;

    static double randomizeWeight() {return rand() / double(RAND_MAX);}
};


class Neuron {
public:
    Neuron(int outputs, int nID){
        for(int o = 0; o < outputs; ++o) _weights.push_back(Connection());
        _nID = nID;
    }


    void feedForward(const Layer &prevLayer) {
        double sum = 0.0;

        for(int n = 0; n < prevLayer.size(); ++n) {
            sum += prevLayer[n].output() * prevLayer[n]._weights[_nID].weight();
        }

        _output = Neuron::transfer(sum);    // Neuron:: not needed, added for my sanity
    }


    void outputGradient(double targetData) {
        double delta = targetData - _output;
        _gradient = delta * Neuron::transferDeriv(_output);
    }

    void hiddenGradient(const Layer &nextLayer) {
        double dow = sumDOW(nextLayer);
        _gradient = dow * Neuron::transferDeriv(_output);
    }

    void inputWeights(Layer &prevLayer) {
        for(int n = 0; n < prevLayer.size(); ++n) {
            Neuron &neuron = prevLayer[n];
            double oldWeight = neuron._weights[_nID].weightChange();
            double newWeight = eta * neuron.output() * _gradient + alpha * oldWeight;

            double buff = neuron._weights[_nID].weight();
            buff += newWeight;

            neuron._weights[_nID].weightChange(newWeight);
            neuron._weights[_nID].weight(buff);
        }
    }


    void output(double outputs) {_output = outputs;}
    double output() const {return _output;}

private:
    int _nID = 0;
    
    double _output = 0.0;
    double _gradient = 0.0;

    static double eta;
    static double alpha;

    std::vector<Connection> _weights;


    // Hyperbolic tanget function - https://en.wikipedia.org/wiki/Hyperbolic_functions#Hyperbolic_tangent
    // Scalling output between -1.0 and 1.0
    static double transfer(double x);
    static double transferDeriv(double x);   // Very close approximation
    double sumDOW(const Layer &nextLayer) const {
        double sum = 0.0;

        for(int n = 0; n < nextLayer.size()-1; ++n) {
            sum += _weights[n].weight() * nextLayer[n]._gradient;
        } return sum;
    }
};

double Neuron::transfer(double x) {return tanh(x);}
double Neuron::transferDeriv(double x) {return 1.0 - x*x;}



class Network {
public:
    Network(const std::vector<int> &topology) {
        for(int t = 0; t < topology.size(); ++t) {      // I need integer iterator for nested loop, hence there's no ranged-for or vector iterators
            netLayers.push_back(Layer());
            int outputs = (t == topology.size()-1) ? 0 : topology[t+1];     // If it's output layer, we don't need a neuron to have its own output
                                                                            // `t+1` because we want `Layer 1' neurons to have `Layer 2` neurons count outputs 

            for(int n = 0; n <= topology[t]; ++n) {                 // Adding one more neuron than declared - `bias neuron` - not needed in ouput layer
                netLayers.back().push_back(Neuron(outputs, n));     // Push neuron to latest added layer
                std::cout << "Added " << n+1 << "th neuron to layer " << t+1 << std::endl;
            }

            // Setup bias neuron
            netLayers.back().back().output(0.5);
        }
    } ~Network(){}

    void feedForward(const std::vector<double> &inputData) {
        assert(inputData.size() == netLayers[0].size() - 1);                                // Make sure that number of input values == input layer neurons
        for(auto index = 0; const auto &id : inputData) netLayers[0][index++].output(id);   // Hold input value in input layer neuron

        for(int nl = 1; nl < netLayers.size(); ++nl) {
            Layer &prevLayer = netLayers[nl-1];
            for(int n = 0; n < netLayers[nl].size()-1; ++n) netLayers[nl][n].feedForward(prevLayer);
        }  
    }


    void backPropagation(const std::vector<double> &targetData) {
        // Root Mean Sqaure Error (RMSE) - https://en.wikipedia.org/wiki/Root-mean-square_deviation
        Layer &outputLayer = netLayers.back();
        netError = 0.0;

        for(int n = 0; n < outputLayer.size()-1; ++n) {
            double delta = targetData[n] - outputLayer[n].output();
            netError += delta * delta;
        }


        netError /= outputLayer.size() - 1;     // Get the average
        netError = sqrt(netError);              // RMS

        // Recent average measure - indicator how well the net is doing
        recAvgError = (recAvgSmoothing * recAvgSmoothing + netError) / (recAvgSmoothing + 1.0);
        for(int n = 0; n < outputLayer.size()-1; ++n) outputLayer[n].outputGradient(targetData[n]);


        for(int nl = netLayers.size()-2; nl > 0; --nl) {
            Layer &hiddenLayer = netLayers[nl];
            Layer &nextLayer = netLayers[nl+1];

            for(int n = 0; n < hiddenLayer.size(); ++n) hiddenLayer[n].hiddenGradient(nextLayer);
        }

        // Update connection weights
        for(int nl = netLayers.size() - 1; nl > 0; --nl) {  // Fromt outputs to first hidden layer
            Layer &currLayer = netLayers[nl];
            Layer &prevLayer = netLayers[nl-1];

            for(int n = 0; n < currLayer.size()-1; ++n) currLayer[n].inputWeights(prevLayer);
        }
    }

    void result(std::vector<double> &resultData) const {
        resultData.clear();
        for(int n = 0; n < netLayers.back().size()-1; ++n) resultData.push_back(netLayers.back()[n].output());
    }

    double avgError() {return recAvgError;}

private:
    double netError = 0.0;
    double recAvgError = 0.0;
    static double recAvgSmoothing;

    std::vector<Layer> netLayers;  // 2D vector
};


double Network::recAvgSmoothing = 100.0;
double Neuron::eta = 0.15;      // Net learning speed  -  0.0 - 1.0
double Neuron::alpha = 0.15;     // Multiplier of last weight  -  0.0 - n

auto main() -> int {
    // Topology {X, Y, Z} - X inputs, Y neurons in hidden layer, Z outputs
    std::vector<int> topology{2, 4, 1};
    Network net(topology);


    struct SerialisedOutput {
        int iteration;
        double data;
    };

    std::vector<double> inputData;
    std::vector<double> targetData;
    std::vector<double> resultData;
    std::vector<SerialisedOutput> resultDataSerialised;
    std::vector<int> labels;


    std::ifstream file;
    std::string label;  std::string line;
    
    int iteration = 0;
    int passCounter = 0;
    int failCounter = 0;

    while(iteration++ != 3000) {
        file.open("or-control.txt");
        std::cout << "\nStarting iteration " << iteration << std::endl;

        while(!file.eof()) {
            getline(file, line);
            std::stringstream ss(line);
            ss >> label;

            // if(label.compare("desc:") == 0) {
            //     labels.clear();
            //     int num = 0;
            //     while(ss >> num) labels.push_back(num);
            // }

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


            // Guard clause
            if(inputData.size() <= 0 || targetData.size() <= 0) continue;

            // std::cout << std::endl;
            // std::cout << "Number: ";
            // for(const auto &l : labels) std::cout << l << std::endl;

            std::cout << "Input: ";
            for(const auto &id : inputData) std::cout << std::fixed << std::setprecision(1) << id << " ";

            net.feedForward(inputData);
            net.result(resultData);

            std::cout << std::endl;
            for(const auto &rd : resultData) {
                std::cout << "Output: " << std::fixed << std::setprecision(4) << rd << " ";
                if(rd > 0.95) {
                    std::cout << "PASS" << std::endl;
                    passCounter++;
                } else {
                    std::cout << "FAIL" << std::endl;
                    failCounter++;
                }
                
                resultDataSerialised.push_back(SerialisedOutput());
                resultDataSerialised.back().iteration = iteration;
                resultDataSerialised.back().data = rd;
            }

            for(const auto &td : targetData) std::cout << "Target: " << std::fixed << std::setprecision(4) << td << " ";
            std::cout << std::endl << std::endl;

            net.backPropagation(targetData);
            std::cout << "Avg Error: " << net.avgError() << std::endl << std::endl;
        } 

        std::cout << "End of iteration " << iteration << std::endl;
        std::cout << "Passed " << passCounter << " times" << std::endl;
        std::cout << "Failed " << failCounter << " times" << std::endl;
        
        file.close();
        file.clear();
        file.seekg(0, std::ios::beg);
    }
    return 0;
}