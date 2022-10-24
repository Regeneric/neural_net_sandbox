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

#define FILE "net-control.lua"


bool checkL(lua_State *lua, int l) {
    if(l != LUA_OK) {
        std::string err = lua_tostring(lua, -1);
        std::cout << err << std::endl;
        return false;
    } return true;
}

template <class T>
void luaTable(lua_State *lua, std::vector<T> &table) {
    lua_pushvalue(lua, -1);
    lua_pushnil(lua);

    while(lua_next(lua, -2)) {
        lua_pushvalue(lua, -2);
        int val = lua_tointeger(lua, -2);
        table.push_back((T)val);
        lua_pop(lua, 2);
    } lua_pop(lua, 1);

    return;
}

template <class K, class V>
void pushLuaTable(lua_State *lua, K key, V &value) {
    lua_pushinteger(lua, key);
    lua_pushnumber(lua, value);
    lua_settable(lua, -3);

    return;
}


auto main() -> int {
    std::vector<int> topology;
    
    std::vector<double> inputData;
    std::vector<double> targetData;
    std::vector<double> resultData;
    
    int iterations  = 0;
    double threshold = 0.0;
    

    lua_State *lua = luaL_newstate();
    luaL_openlibs(lua);

    if(!checkL(lua, luaL_dofile(lua, FILE)));
    
    lua_getglobal(lua, "setup");
    if(lua_isfunction(lua, -1)) {
        if(checkL(lua, lua_pcall(lua, 0, 0, 0)));
    }

    lua_getglobal(lua, "topology");
    if(lua_istable(lua, -1)) luaTable(lua, topology);

    lua_getglobal(lua, "inputData");
    if(lua_istable(lua, -1)) luaTable(lua, inputData);

    lua_getglobal(lua, "targetData");
    if(lua_istable(lua, -1)) luaTable(lua, targetData);

    lua_getglobal(lua, "iterations");
    if(lua_isnumber(lua, -1)) iterations = lua_tointeger(lua, -1);

    lua_getglobal(lua, "threshold");
    if(lua_isnumber(lua, -1)) threshold = lua_tointeger(lua, -1);

    Network net(topology);
    

    int indHelper = 0;
    int tgtIndHelper = 0;

    int currIteration = 0;
    while(currIteration++ != iterations) {
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


            for(auto index = 1; const auto &rd : resultData) {
                lua_getglobal(lua, "resultData");
                pushLuaTable(lua, index++, rd);
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

            net.backPropagation(tgtBuff);
        } 

        std::cout << std::endl << "Avg Error: " << net.avgError() << std::endl;
        std::cout <<  "End of iteration " << currIteration << std::endl;
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