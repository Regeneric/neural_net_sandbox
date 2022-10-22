#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <utility>
#include <iterator>
#include <memory>
#include <algorithm>

#include <cstdlib>
#include <cassert>
#include <cmath>


#define Rect sf::RectangleShape
#define R_WIDTH 50
#define R_HEIGHT 50


#define TRESHOLD    0.90
#define ITERATIONS  20000
#define ETA         0.15
#define ALPHA       0.50
#define SMOOTHING   100.0
#define FILE        "nums.txt"
#define OFFSET      15


class Neuron;
typedef std::vector<Neuron> Layer;


struct TrainedWeightsTrsh {
    int id;
    std::vector<double> data; 
};


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

        // if(knownWeights.size() > 0) std::copy(knownWeights.begin(), knownWeights.end(), std::back_inserter(_knownWeights));
    }

    int id() {return _nID;}
    double weight() const {return _weights[_nID].weight();}

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

            // if(_knownWeights.size() > 0) {
                // if(n <= 0) neuron._weights[_nID].weight(_knownWeights[n]);
                // else neuron._weights[_nID].weight(_knownWeights[n+OFFSET]);
            // }
            // else {
                double oldWeight = neuron._weights[_nID].weightChange();
                double newWeight = eta * neuron.output() * _gradient + alpha * oldWeight;

                double buff = neuron._weights[_nID].weight();
                buff += newWeight;

                neuron._weights[_nID].weightChange(newWeight);
                neuron._weights[_nID].weight(buff);   
            // }
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

    // std::vector<double> _knownWeights;
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
    Network(const std::vector<int> &topology, std::vector<Layer> knownWeights = std::vector<Layer>()) {
        for(int t = 0; t < topology.size(); ++t) {      // I need integer iterator for nested loop, hence there's no ranged-for or vector iterators
            netLayers.push_back(Layer());
            int outputs = (t == topology.size()-1) ? 0 : topology[t+1];     // If it's output layer, we don't need a neuron to have its own output
                                                                            // `t+1` because we want `Layer 1' neurons to have `Layer 2` neurons count outputs 

            for(int n = 0; n <= topology[t]; ++n) {     // Adding one more neuron than declared - `bias neuron` - not needed in ouput layer
                // if(knownWeights.size() > 0) netLayers.back().push_back(Neuron(outputs, n, knownWeights));     // Push neuron to latest added layer and pass known, good weights
                if(knownWeights.size() > 0) std::copy(netLayers.begin(), netLayers.end(), std::back_inserter(knownWeights));
                netLayers.back().push_back(Neuron(outputs, n));                                          // Push neuron to latest added layer
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

    void result(std::vector<double> &resultData, std::vector<Layer> &trainedWeights) {
        resultData.clear();
        for(int n = 0; n < netLayers.back().size()-1; ++n) {
            resultData.push_back(netLayers.back()[n].output());
            
            // if(netLayers.back()[n].output() >= TRESHOLD) {
                // !!! C++20 !!!
                for(auto index = 0; auto &nl : netLayers) {
                    if(index++ == netLayers.size()-1) break;
                    if(trainedWeights.size() > 100) trainedWeights.clear();
                    trainedWeights.push_back(nl);
                }
            // }
        
            // if(netLayers.back()[n].output() >= TRESHOLD) {
            //     for(auto index = 0; const auto &nl : netLayers) {
            //         if(index++ == netLayers.size()-1) break;

            //         trainedWeights.push_back(TrainedWeightsTrsh());
            //         trainedWeights.back().id = n;
            //         trainedWeights.back().data.push_back(nl.back().weight());
            //     }
            // }
        }
    }

    double avgError() {return recAvgError;}

private:
    double netError = 0.0;
    double recAvgError = 0.0;
    static double recAvgSmoothing;

    std::vector<Layer> netLayers;  // 2D vector
};


double Network::recAvgSmoothing = SMOOTHING;
double Neuron::eta = ETA;       // Net learning speed  -  0.0 - 1.0
double Neuron::alpha = ALPHA;   // Multiplier of last weight  -  0.0 - n

auto main() -> int {
    // Topology {X, Y, Z} - X inputs, Y neurons in hidden layer, Z outputs
    // std::vector<int> topology{2, 4, 1};
    std::vector<int> topology{15, 15, 10};

    // std::ifstream kwni("./known-weight-nums.bin", std::ios::binary);
    // kwni.unsetf(std::ios::skipws);
    
    // std::streampos fileSize;
    //     kwni.seekg(0, std::ios::end);
    //     fileSize = kwni.tellg();
    //     kwni.seekg(0, std::ios::beg);
    // std::vector<Layer> knownWeights; knownWeights.resize(fileSize);
    // for(int i = 0; i < fileSize; i++) kwni.read((char *) &knownWeights[i], sizeof(Neuron)); kwni.close();
    // knownWeights.insert(knownWeights.begin(), 
    //                     std::istream_iterator<Layer>(kwni),
    //                     std::istream_iterator<Layer>());
    // if(!kwni.good()) std::cout << "[ERROR]" << " Read from file has failed!" << std::endl;
    

    // Network net(topology, knownWeights);
    Network net(topology);


    std::vector<double> inputData;
    std::vector<double> targetData;
    std::vector<double> resultData;

    std::vector<std::string> labels;
    std::vector<Layer> trainedWeights;


    std::ifstream file;
    std::string label;  std::string line;
    
    int iteration = 0;
    int passCounter = 0;
    int failCounter = 0;

    int cnt = 0;
    while(iteration++ != ITERATIONS) {
        file.open(FILE);
        std::cout << "\nStarting iteration " << iteration << std::endl;

        while(!file.eof()) {
            getline(file, line);  cnt++;
            std::stringstream ss(line);
            ss >> label;

            if(label.compare("desc:") == 0) {
                labels.clear();
                std::string desc;
                while(ss >> desc) labels.push_back(desc);
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

            if(inputData.size() <= 0 || targetData.size() <= 0) continue;   // Guard clause


            std::cout << std::endl << "Number: ";
            for(const auto &l : labels) std::cout << l << std::endl;

            std::cout << "Input: ";
            for(const auto &id : inputData) std::cout << std::fixed << std::setprecision(1) << id << " ";

            net.feedForward(inputData);
            net.result(resultData, trainedWeights);

            std::cout << std::endl;
            for(const auto &rd : resultData) {
                std::cout << "Output: " << std::fixed << std::setprecision(4) << rd << " ";
                if(rd >= TRESHOLD) {
                    std::cout << "PASS" << std::endl;
                    passCounter++;
                } else {
                    std::cout << "FAIL" << std::endl;
                    failCounter++;
                }
            }


            for(const auto &td : targetData) std::cout << "Target: " << std::fixed << std::setprecision(4) << td << " " << std::endl;;
            std::cout << std::endl;

            net.backPropagation(targetData);
            std::cout << "Avg Error: " << net.avgError() << std::endl << std::endl;

            inputData.clear();
            targetData.clear();
        } 

        std::cout << "End of iteration " << iteration << std::endl;
        
        file.close();
        file.clear();
        file.seekg(0, std::ios::beg);
    }

    std::cout << "Passed " << passCounter << " times" << std::endl;
    std::cout << "Failed " << failCounter << " times" << std::endl;

    
    // std::ofstream kwno("./known-weight-nums.bin", std::ios::binary);
    // kwno.unsetf(std::ios::skipws);
    // for(const auto &tw : trainedWeights) kwno.write((char *) &tw, sizeof(Neuron)); kwno.close();
    // if(!kwno.good()) std::cout << "[ERROR] Writing to file has failed" << std::endl;


    std::cout << std::endl << "Trained weights: " << std::endl;
    for(int row = 0; row < trainedWeights.size()-1; ++row) {
        std::cout << std::left << std::setw(2) << row << ":  ";

        for(int col = 0; col < trainedWeights[row].size()-1; ++col) {
            std::cout << std::left << std::setw(8) << trainedWeights[row][col].weight() << std::left << std::setw(1) << ", ";
        } 
        
        std::cout << std::endl;
    }


    sf::RenderWindow app;
        app.create(sf::VideoMode(800, 600, 32), "Retina");
        app.setFramerateLimit(60);

    int xPos = 25;
    int yPos = 25;

    sf::Color originalColor(211, 211, 211);
    std::vector<std::vector<Rect>> retina;  retina.resize(150);

    for(int i = 0; i != 5; ++i) {
        for(int j = 0; j != 3; ++j) {
            Rect field;
                field.setPosition(xPos, yPos);
                field.setSize(sf::Vector2f(R_WIDTH, R_HEIGHT));
                field.setFillColor(originalColor);
                field.setOutlineThickness(2.0f);
                field.setOutlineColor(sf::Color::Black);

            retina[i].push_back(field);
            xPos += R_WIDTH;
        } 
        
        yPos += R_HEIGHT;
        xPos = 25;
    }


    Rect resetButton;
        resetButton.setPosition(xPos, yPos + R_HEIGHT);
        resetButton.setSize(sf::Vector2f((R_WIDTH*2), R_HEIGHT));
        resetButton.setFillColor(sf::Color::Red);
        resetButton.setOutlineThickness(2.0f);
        resetButton.setOutlineColor(sf::Color::Black);

    Rect resultButton;
        resultButton.setPosition(xPos+(R_WIDTH*3), yPos + R_HEIGHT);
        resultButton.setSize(sf::Vector2f((R_WIDTH*2), R_HEIGHT));
        resultButton.setFillColor(sf::Color::Green);
        resultButton.setOutlineThickness(2.0f);
        resultButton.setOutlineColor(sf::Color::Black);


    auto mouse = std::make_unique<sf::Mouse>();
    int clicked = 0;
    int i = 15;

    sf::Font font;
    font.loadFromFile("arial.ttf");

    sf::Text txt;
        txt.setCharacterSize(32);
        txt.setFillColor(sf::Color::Black);
        txt.setFont(font);


    inputData.clear();
    inputData.resize(5*3);
    while(app.isOpen()) {
        sf::Event event;
        while(app.pollEvent(event)) {
            if(event.type == sf::Event::Closed) app.close();
        } app.clear(sf::Color(230, 230, 230));

    
        float mouseX = mouse->getPosition(app).x;
        float mouseY = mouse->getPosition(app).y;
        
        if(sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            i = 15;
            while(i != 0) --i;

            clicked = 1;
        } else clicked = 0;

        app.draw(resetButton);
        if(resetButton.getGlobalBounds().contains(sf::Vector2f(mouseX, mouseY))) {
            if(clicked) {
                for(int i = 0; i != 5; ++i) {
                    for(int j = 0; j != 3; ++j) {
                        retina[i][j].setFillColor(originalColor);   
                        inputData[i*3+j] = 0.0;       
                    }
                }
            }
        }


        for(int i = 0; i < 5; i++) {
            for(int j = 0; j < 3; j++) {
                if(retina[i][j].getGlobalBounds().contains(sf::Vector2f(mouseX, mouseY))) {
                    if(clicked) {
                        retina[i][j].setFillColor(sf::Color::Black);
                        inputData[i*3+j] = 1.0;
                    }
                } app.draw(retina[i][j]);            
            }
        }

        net.feedForward(inputData);
        net.result(resultData, trainedWeights);

        app.draw(resultButton);
        if(resultButton.getGlobalBounds().contains(sf::Vector2f(mouseX, mouseY))) {
            if(clicked) {
                double key = *std::max_element(resultData.begin(), resultData.end());
                auto num = std::find(resultData.begin(), resultData.end(), key);

                if(num != resultData.cend())
                    std::cout << "Most probable number on the screen: " << std::distance(resultData.begin(), num) << std::endl;
            }
        }

        app.display();
    }







    // int kgwCnt = 0;
    // std::cout << std::endl << "Trained, good weights: " << std::endl;
    // for(int row = trainedWeights.size()-1; row >= 0; --row) {
    //     std::cout << std::endl << std::left << std::setw(8) << "Input: ";
    //     for(int tw = 0; tw < trainedWeights.back().size()-1; ++tw) {
    //         std::cout << trainedWeights[row][tw].id() << " ";
    //         std::cout << std::left << std::setw(8) << trainedWeights[row][tw].weight() << std::setw(1) << ", ";
    //     } 
    // } std::cout << std::endl;

    // int kgwCnt = 0;
    // std::cout << std::endl << "Known, good weights: " << std::endl;
    // for(int row = trainedWeights.size()-1; row > trainedWeights.size()-3; --row) {
    //     std::cout << std::endl << std::left << std::setw(7) << "Input " << kgwCnt++ << " ";
    //     for(int tw = 0; tw < trainedWeights[row].size()-1; ++tw) {
    //         std::cout << std::left << std::setw(7) << trainedWeights[row][tw].weight();
    //     }
    // } std::cout << std::endl;

    return 0;
}