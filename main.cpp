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

extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}


#include "headers/Connection.hpp"
#include "headers/Neuron.hpp"
#include "headers/Network.hpp"


#define Rect sf::RectangleShape
#define R_WIDTH 50
#define R_HEIGHT 50


#define TRESHOLD    0.90
#define ITERATIONS  20000
#define FILE        "./datasets/nums.data"
#define OFFSET      15


auto main() -> int {
    // std::vector<int> topology{2, 4, 1};
    std::vector<int> topology{15, 15, 10};
    Network net(topology);

    std::vector<double> inputData;
    std::vector<double> targetData;
    std::vector<double> resultData;

    std::vector<std::string> labels;
    std::vector<Layer> trainedWeights;

    
    int iteration = 0;
    int passCounter = 0;
    int failCounter = 0;


    std::ifstream file;
    std::string label;   std::string line;

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

    return 0;
}