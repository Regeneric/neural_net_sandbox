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


#include "headers/commons.hpp"

#include "headers/Connection.hpp"
#include "headers/Neuron.hpp"
#include "headers/Network.hpp"

#define FILE "net-config.lua"


auto main() -> int {
    bool useRetina = false;

    lua_State *lua = luaL_newstate();
    luaL_openlibs(lua);

    if(!checkL(lua, luaL_dofile(lua, FILE)));
    lua_getglobal(lua, "useRetina");
    if(lua_isboolean(lua, -1)) useRetina = lua_toboolean(lua, -1);


    if(useRetina) {
        sf::RenderWindow app;
            app.create(sf::VideoMode(250, 250, 32), "Retina", sf::Style::Default);
            app.setFramerateLimit(100);
            app.setVerticalSyncEnabled(false);

        
        // Setting up canvas - whiteboard to draw letters
        sf::RenderTexture canvas;
            canvas.create(250, 250);
            canvas.clear(sf::Color::White);

        sf::Sprite sprite;
            sprite.setTexture(canvas.getTexture(), true);

        const float brushSize = 5;
        sf::Color brushColor = sf::Color::Black;
        // const std::vector<sf::Color> colors{
        //     sf::Color(255, 0, 0, 8),
        //     sf::Color(255, 255, 0, 8),
        //     sf::Color(0, 255, 0, 8),
        //     sf::Color(0, 255, 255, 8),
        //     sf::Color(0, 0, 255, 8),
        //     sf::Color(255, 0, 255, 8)
        // };
        sf::CircleShape brush(brushSize, 24);
            brush.setFillColor(brushColor);

        sf::Vector2f lastPos;
        bool isDrawing = false;
        // int color = 0;


        while(app.isOpen()) {
            sf::Event event;
            while(app.pollEvent(event)) {
                switch(event.type) {
                    case sf::Event::Closed: app.close(); break;
                    case sf::Event::Resized: {
                        const sf::Vector2f size(app.getSize().x, app.getSize().y);
                        sf::View view(app.getView());
                            view.setSize(size);
                            view.setCenter(size/2.f);
                            app.setView(view);
                    } break;
                    case sf::Event::KeyPressed: {
                        switch(event.key.code) {
                            case sf::Keyboard::C: {
                                canvas.clear(sf::Color::White);
                                canvas.display();
                            } break;
                        }
                    } break;
                    case sf::Event::MouseButtonPressed: {
                        if(event.mouseButton.button == sf::Mouse::Left) {
                            isDrawing = true;
                            lastPos = app.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
                            
                            brush.setPosition(lastPos);
                            canvas.draw(brush);
                            canvas.display();
                        }
                    } break;
                    case sf::Event::MouseButtonReleased: {
                        if(event.mouseButton.button == sf::Mouse::Left) isDrawing = false;
                    } break;
                    case sf::Event::MouseMoved: {
                        if(isDrawing) {
                            const sf::Vector2f newPos(app.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y)));
                            
                            brush.setPosition(newPos);
                            canvas.draw(brush);
                            canvas.display();
                        }
                    } break;
                }
            } app.clear(sf::Color(230, 230, 230));

            app.draw(sprite);
            app.display();
        }
    }
    


    std::vector<int> topology;
    
    std::vector<double> inputData;
    std::vector<double> targetData;
    std::vector<double> resultData;
    std::vector<KNOWN_WEIGHTS> trainedWeights;

    bool useTrained = false;
    std::string trained;

    int iterations  = 0;
    double threshold = 0.0;


    lua_getglobal(lua, "setup");
    if(lua_isfunction(lua, -1)) {
        if(checkL(lua, lua_pcall(lua, 0, 0, 0)));
    }

    lua_getglobal(lua, "topology");
    if(lua_istable(lua, -1)) luaTable(lua, topology);

    if(!useRetina) {
        lua_getglobal(lua, "inputData");
        if(lua_istable(lua, -1)) luaTable(lua, inputData);
    }

    lua_getglobal(lua, "targetData");
    if(lua_istable(lua, -1)) luaTable(lua, targetData);

    lua_getglobal(lua, "iterations");
    if(lua_isnumber(lua, -1)) iterations = lua_tointeger(lua, -1);

    lua_getglobal(lua, "threshold");
    if(lua_isnumber(lua, -1)) threshold = lua_tointeger(lua, -1);

    lua_getglobal(lua, "trainedWeightsFile");
    if(lua_isstring(lua, -1)) trained = lua_tostring(lua, -1);

    lua_getglobal(lua, "useTrainedWeights");
    if(lua_isboolean(lua, -1)) useTrained = lua_toboolean(lua, -1);


    if(useTrained) {
        std::ifstream ktwi;
            ktwi.open(trained, std::ios::in);
            while(!ktwi.eof()) {
                KNOWN_WEIGHTS dummy;
                ktwi.read((char*)&dummy, sizeof(dummy));
                trainedWeights.push_back(dummy);
            }
    }

    Network net(topology);

    int currIteration = 0;
    while(currIteration++ != iterations) {
        if(!useTrained) trainedWeights.clear();
        int iter = 0;
        std::cout << "\nStarting iteration " << currIteration << std::endl; 

        int inpDivider = topology.front();     // We don't number of inputs during compilation
        int iterTo = inputData.size()/inpDivider-1;

        std::vector<double> inpBuff;
        std::vector<double> tgtBuff; 

        for(int i = 0; i <= iterTo; i++) {
            // topology.front() == number of inputs
            int offsetBgn = topology.front()*i;
            int offsetEnd = topology.front()*(i+1);

            inpBuff.clear();
            std::copy(inputData.begin()+offsetBgn, 
                      inputData.begin()+offsetEnd, 
                      std::back_inserter(inpBuff));

            net.feedForward(inpBuff);
            net.result(resultData);


            lua_getglobal(lua, "resultData");
            for(auto index = 1; const auto &rd : resultData) {
                lua_checkstack(lua, resultData.size());
                pushLuaTable(lua, index++, rd, targetData.size());
            }


            lua_getglobal(lua, "display");
            if(lua_isfunction(lua, -1)) { 
                lua_pushnumber(lua, i+1);
                lua_pushnumber(lua, topology.front()-1);
                if(checkL(lua, lua_pcall(lua, 2, 0, 0)));
            } 
            

            // topology.back() == number of outputs
            offsetBgn = topology.back()*i;
            offsetEnd = topology.back()*(i+1);

            tgtBuff.clear();
            std::copy(targetData.begin()+offsetBgn,
                      targetData.begin()+offsetEnd,
                      std::back_inserter(tgtBuff));

            net.backPropagation(tgtBuff, trainedWeights, iter, useTrained);
        } 

        std::cout << std::endl << "Avg Error: " << net.avgError() << std::endl;
        std::cout <<  "End of iteration " << currIteration << std::endl;
    }

    if(!useTrained) {
        std::ofstream two;
            two.open(trained, std::ios::binary);
            for(const auto &tw : trainedWeights) two.write((char*)&tw, sizeof(tw));
            two.close();
    }

    
    // sf::RenderWindow app;
    //     app.create(sf::VideoMode(800, 600, 32), "Retina");
    //     app.setFramerateLimit(60);

    // int xPos = 25;
    // int yPos = 25;

    // sf::Color originalColor(211, 211, 211);
    // std::vector<std::vector<Rect>> retina;  retina.resize(150);

    // for(int i = 0; i != 5; ++i) {
    //     for(int j = 0; j != 3; ++j) {
    //         Rect field;
    //             field.setPosition(xPos, yPos);
    //             field.setSize(sf::Vector2f(R_WIDTH, R_HEIGHT));
    //             field.setFillColor(originalColor);
    //             field.setOutlineThickness(2.0f);
    //             field.setOutlineColor(sf::Color::Black);

    //         retina[i].push_back(field);
    //         xPos += R_WIDTH;
    //     } 
        
    //     yPos += R_HEIGHT;
    //     xPos = 25;
    // }


    // Rect resetButton;
    //     resetButton.setPosition(xPos, yPos + R_HEIGHT);
    //     resetButton.setSize(sf::Vector2f((R_WIDTH*2), R_HEIGHT));
    //     resetButton.setFillColor(sf::Color::Red);
    //     resetButton.setOutlineThickness(2.0f);
    //     resetButton.setOutlineColor(sf::Color::Black);

    // Rect resultButton;
    //     resultButton.setPosition(xPos+(R_WIDTH*3), yPos + R_HEIGHT);
    //     resultButton.setSize(sf::Vector2f((R_WIDTH*2), R_HEIGHT));
    //     resultButton.setFillColor(sf::Color::Green);
    //     resultButton.setOutlineThickness(2.0f);
    //     resultButton.setOutlineColor(sf::Color::Black);


    // auto mouse = std::make_unique<sf::Mouse>();
    // int clicked = 0;
    // int i = 15;

    // sf::Font font;
    // font.loadFromFile("arial.ttf");

    // sf::Text txt;
    //     txt.setCharacterSize(32);
    //     txt.setFillColor(sf::Color::Black);
    //     txt.setFont(font);


    // inputData.clear();
    // inputData.resize(5*3);
    // while(app.isOpen()) {
    //     sf::Event event;
    //     while(app.pollEvent(event)) {
    //         if(event.type == sf::Event::Closed) app.close();
    //     } app.clear(sf::Color(230, 230, 230));

    
    //     float mouseX = mouse->getPosition(app).x;
    //     float mouseY = mouse->getPosition(app).y;
        
    //     if(sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
    //         i = 15;
    //         while(i != 0) --i;

    //         clicked = 1;
    //     } else clicked = 0;

    //     app.draw(resetButton);
    //     if(resetButton.getGlobalBounds().contains(sf::Vector2f(mouseX, mouseY))) {
    //         if(clicked) {
    //             for(int i = 0; i != 5; ++i) {
    //                 for(int j = 0; j != 3; ++j) {
    //                     retina[i][j].setFillColor(originalColor);   
    //                     inputData[i*3+j] = 0.0;       
    //                 }
    //             }
    //         }
    //     }


    //     for(int i = 0; i < 5; i++) {
    //         for(int j = 0; j < 3; j++) {
    //             if(retina[i][j].getGlobalBounds().contains(sf::Vector2f(mouseX, mouseY))) {
    //                 if(clicked) {
    //                     retina[i][j].setFillColor(sf::Color::Black);
    //                     inputData[i*3+j] = 1.0;
    //                 }
    //             } app.draw(retina[i][j]);            
    //         }
    //     }

    //     net.feedForward(inputData);
    //     net.result(resultData, trainedWeights);

    //     app.draw(resultButton);
    //     if(resultButton.getGlobalBounds().contains(sf::Vector2f(mouseX, mouseY))) {
    //         if(clicked) {
    //             double key = *std::max_element(resultData.begin(), resultData.end());
    //             auto num = std::find(resultData.begin(), resultData.end(), key);

    //             if(num != resultData.cend())
    //                 std::cout << "Most probable number on the screen: " << std::distance(resultData.begin(), num) << std::endl;
    //         }
    //     }

    //     app.display();
    // }

    return 0;
}
