#include "luabidings.h"
#include "dataserver.h"
#include "udpserver.h"
#include "mysql.h"
extern "C" {
#include "thirdparty/lua/lua.h"
#include "thirdparty/lua/lualib.h"
#include "thirdparty/lua/lauxlib.h"
}
#ifndef LUA_OK
#define LUA_OK 0
#endif
#include <iostream>
lua_State* L = nullptr;

int lua_SQLFree(lua_State* L)
{
    DataServer::MarkQueryCompleted();
    return 0;
}
int lua_LogText(lua_State* L)
{
    const char* msg = luaL_checkstring(L, 1);
    printf("[Lua] %s\n", msg);
    return 0;
}
int lua_SendPacket(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    const char* param = luaL_checkstring(L, 2);
    const char* resultStr = luaL_checkstring(L, 3);

    UDPServer::SendResult(label, param, resultStr); // <<<<< AGORA OK: passa string
    return 0;
}


// SQLConnect(odbc, user, pass)
int lua_SQLConnect(lua_State* L) {
    const char* odbc = luaL_checkstring(L, 1);
    const char* user = luaL_checkstring(L, 2);
    const char* pass = luaL_checkstring(L, 3);
    bool success = SQLSystem::Connect(odbc, user, pass);
    lua_pushboolean(L, success);
    return 1;
}

// SQLDisconnect()
int lua_SQLDisconnect(lua_State* L) {
    SQLSystem::Disconnect();
    return 0;
}

// SQLQuery(query)
int lua_SQLQuery(lua_State* L) {
    const char* query = luaL_checkstring(L, 1);
    bool success = SQLSystem::Query(query);
    lua_pushboolean(L, success);
    return 1;
}

// SQLFetch()
int lua_SQLFetch(lua_State* L) {
    bool success = SQLSystem::Fetch();
    lua_pushboolean(L, success);
    return 1;
}

// SQLClose()
int lua_SQLClose(lua_State* L) {
    SQLSystem::Close();
    return 0;
}

// SQLGetNumber(col)
int lua_SQLGetNumber(lua_State* L) {
    int col = luaL_checkinteger(L, 1);
    int value = SQLSystem::GetNumber(col);
    lua_pushinteger(L, value);
    return 1;
}

// SQLGetString(col)
int lua_SQLGetString(lua_State* L) {
    int col = luaL_checkinteger(L, 1);
    std::string value = SQLSystem::GetString(col);
    lua_pushstring(L, value.c_str());
    return 1;
}

// SQLGetFloat(col)
int lua_SQLGetFloat(lua_State* L) {
    int col = luaL_checkinteger(L, 1);
    float value = SQLSystem::GetFloat(col);
    lua_pushnumber(L, value);
    return 1;
}

// SQLGetResult()
int lua_SQLGetResult(lua_State* L) {
    lua_pushboolean(L, SQLSystem::GetResult());
    return 1;
}

// SQLIsConnected()
int lua_SQLIsConnected(lua_State* L) {
    lua_pushboolean(L, SQLSystem::IsConnected());
    return 1;
}

void RegisterLuaFunctions(lua_State* L)
{
    lua_register(L, "SQLConnect", lua_SQLConnect);
    lua_register(L, "SQLDisconnect", lua_SQLDisconnect);
    lua_register(L, "SQLQuery", lua_SQLQuery);
    lua_register(L, "SQLFetch", lua_SQLFetch);
    lua_register(L, "SQLClose", lua_SQLClose);
    lua_register(L, "SQLGetNumber", lua_SQLGetNumber);
    lua_register(L, "SQLGetString", lua_SQLGetString);
    lua_register(L, "SQLGetFloat", lua_SQLGetFloat);
    lua_register(L, "SQLGetResult", lua_SQLGetResult);
    lua_register(L, "SQLIsConnected", lua_SQLIsConnected);
    lua_register(L, "LogText", lua_LogText);
    lua_register(L, "SQLFree", lua_SQLFree);           // Já tinha
    lua_register(L, "SendPacket", lua_SendPacket);     // Já tinha
}

void LuaBindings::Init()
{
    L = luaL_newstate();
    luaL_openlibs(L);
    RegisterLuaFunctions(L);

    if (luaL_dofile(L, "main.lua"))
    {
        printf("[Lua] Erro: %s\n", lua_tostring(L, -1));
    }
    else
    {
        printf("[Lua] Lua carregado com sucesso.\n");
    }
}

void LuaBindings::OnDsQuery(int size, const std::string& query, const std::string& label, const std::string& param)
{
    lua_getglobal(L, "OnDsQuery");

    lua_pushinteger(L, size);
    lua_pushstring(L, query.c_str());
    lua_pushstring(L, label.c_str());
    lua_pushstring(L, param.c_str());

    if (lua_pcall(L, 4, 0, 0) != LUA_OK)
    {
        printf("[Lua] Erro em OnDsQuery: %s\n", lua_tostring(L, -1));
    }
}
