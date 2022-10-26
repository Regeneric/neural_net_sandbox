#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <iostream>
#include <vector>
#include <fstream>

#include "../headers/commons.hpp"

extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}

#define FILE "dc-config.lua"

auto main() -> int {
    bool showOutput = true;

    int framerate = 5;
    int width = 0; int height = 0;
    
    int inputs = 0;
    int offset = 0;
    
    std::string inputFile  = " ";
    std::string outputFile = " ";

    lua_State *lua = luaL_newstate();
    luaL_openlibs(lua);

    if(!checkL(lua, luaL_dofile(lua, FILE)));
    lua_getglobal(lua, "showOutput");
    if(lua_isboolean(lua, -1)) showOutput = lua_toboolean(lua, -1);

    lua_getglobal(lua, "framerate");
    if(lua_isinteger(lua, -1)) framerate = lua_tointeger(lua, -1);

    lua_getglobal(lua, "width");
    if(lua_isinteger(lua, -1)) width = lua_tointeger(lua, -1);

    lua_getglobal(lua, "height");
    if(lua_isinteger(lua, -1)) height = lua_tointeger(lua, -1);

    lua_getglobal(lua, "inputs");
    if(lua_isinteger(lua, -1)) inputs = lua_tointeger(lua, -1);

    lua_getglobal(lua, "offset");
    if(lua_isinteger(lua, -1)) offset = lua_tointeger(lua, -1);

    lua_getglobal(lua, "inputFile");
    if(lua_isstring(lua, -1)) inputFile = lua_tostring(lua, -1);

    lua_getglobal(lua, "outputFile");
    if(lua_isstring(lua, -1)) outputFile = lua_tostring(lua, -1);


    sf::RenderWindow app;
        app.create(sf::VideoMode(width, height, 32), "Retina", sf::Style::Default);
        app.setFramerateLimit(framerate);
        app.setVerticalSyncEnabled(false);

    sf::Texture texture;
        texture.loadFromFile(inputFile);

    sf::Image image; 
    sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setOrigin(0, 0);

    
    std::vector<float> inputData;

    int originX = 0;
    int originY = 0;

    std::cout << "[C++] Starting conversion..." << std::endl;
    while(app.isOpen()) {
        sf::Event event;
        while(app.pollEvent(event)) {
            if (event.type == sf::Event::Closed) app.close();
        } app.clear(sf::Color(255, 255, 255));

        app.draw(sprite);
        image = app.capture();
        
        for(int i = 0; i < app.getSize().y; i++) {
            for(int j = 0; j < app.getSize().x; j++) {
                // int k = app.getSize().x;
                // inputData[i*k+j] = val; 

                sf::Color color = image.getPixel(i, j);
                float val = (color == sf::Color::Black) ? 1.f : 0.f;    
                inputData.push_back(val); // Flattening 2D vector to 1D
            }
        } 

        if(inputs > 1) {
            originX += app.getSize().x;
            originX += offset;
        
            sprite.setOrigin(originX, originY);
            if(originX > app.getSize().x * inputs) app.close();
        } else app.close();

        if(showOutput) app.display();
    } 
    
    std::cout << "[C++] Conversion has ended" << std::endl;
    std::cout << "[C++] Input data size: " << inputData.size() << std::endl;

    lua_getglobal(lua, "inputData");
    if(lua_istable(lua, -1)) {
        for(auto index = 1; const auto &id : inputData) {
            lua_checkstack(lua, inputData.size());
            pushLuaTable(lua, index++, id, inputData.size());
        }
    }

    lua_getglobal(lua, "convert");
    if(lua_isfunction(lua, -1)) {
        if(checkL(lua, lua_pcall(lua, 0, 0, 0)));
    }
}