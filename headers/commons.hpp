#pragma once

#include <iostream>
#include <string>
#include <vector>

extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}


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
void pushLuaTable(lua_State *lua, K key, V &value, int size) {
    // lua_checkstack(lua, size);
    lua_pushinteger(lua, key);
    lua_pushnumber(lua, value);
    lua_settable(lua, -3);

    return;
}