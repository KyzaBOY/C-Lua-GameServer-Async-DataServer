

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "luabidings.h"
#include <iostream>
#include "tcpserver.h"
#include "udpserver.h"

static lua_State* L = nullptr;
#ifndef LUA_OK
#define LUA_OK 0
#endif

void LuaBindings::Init()
{
    L = luaL_newstate();      // Cria o estado Lua
    luaL_openlibs(L);         // Abre as libs padrões

    RegisterLuaFunctions(L); // << AQUI

    // Carrega o script principal
    if (luaL_dofile(L, "main.lua"))
    {
        std::cerr << "[Lua] Erro ao carregar main.lua: " << lua_tostring(L, -1) << std::endl;
    }
    else
    {
        std::cout << "[Lua] main.lua carregado com sucesso." << std::endl;
    }
}

void LuaBindings::Reload()
{
    if (L != nullptr) {
        lua_close(L);  // Encerra o estado anterior
        L = nullptr;
    }

    L = luaL_newstate();      // Novo estado
    luaL_openlibs(L);         // Libs padrões

    LuaBindings::RegisterLuaFunctions(L);  // Reexpõe todas as funções C++ para Lua

    if (luaL_dofile(L, "main.lua") != LUA_OK)
    {
        std::cerr << "[Lua] ❌ Erro ao recarregar main.lua: " << lua_tostring(L, -1) << std::endl;
    }
    else
    {
        std::cout << "[Lua] ✅ main.lua recarregado com sucesso!" << std::endl;
    }
}

int lua_DSQuery(lua_State* L)
{
    const char* query = luaL_checkstring(L, 1);
    const char* label = luaL_checkstring(L, 2);
    const char* param = luaL_optstring(L, 3, ""); // Opcional

    UDPServer::SendQuery(query, label, param);
    return 0;
}

void LuaBindings::OnSQLAsyncResult(const std::string& label, const std::string& param, const std::string& result)
{
    lua_getglobal(L, "OnSQLAsyncResult");
    lua_pushstring(L, label.c_str());
    lua_pushstring(L, param.c_str());
    lua_pushstring(L, result.c_str());

    if (lua_pcall(L, 3, 0, 0) != LUA_OK)
    {
        printf("[Lua] Erro em OnSQLAsyncResult: %s\n", lua_tostring(L, -1));
    }
}

void LuaBindings::OnClientConnect(int cIndex)
{
    lua_getglobal(L, "OnClientConnect");
    if (lua_isfunction(L, -1))
    {
        lua_pushinteger(L, cIndex);
        if (lua_pcall(L, 1, 0, 0) != LUA_OK)
        {
            std::cerr << "[Lua] Erro OnClientConnect: " << lua_tostring(L, -1) << std::endl;
        }
    }
    else
    {
        lua_pop(L, 1); // remove o valor se não for função
    }
}

void LuaBindings::OnPacketRecv(int cIndex, int head, int sub, const char* data, int dataLen)
{
    lua_getglobal(L, "OnPacketRecv");
    lua_pushinteger(L, cIndex);
    lua_pushinteger(L, head);
    lua_pushinteger(L, sub);

    if (dataLen > 0 && data != nullptr)
        lua_pushlstring(L, data, dataLen);
    else
        lua_pushlstring(L, "", 0); // string vazia em caso de pacote vazio

    lua_pushinteger(L, dataLen);

    if (lua_pcall(L, 5, 0, 0) != LUA_OK)
    {
        std::cerr << "[Lua Error] OnPacketRecv: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
    }
}

void LuaBindings::OnClientDisconnect(int cIndex)
{
    lua_getglobal(L, "OnClientDisconnect"); // pega a função Lua
    if (lua_isfunction(L, -1))
    {
        lua_pushinteger(L, cIndex);         // empilha o parâmetro
        if (lua_pcall(L, 1, 0, 0) != 0)
        {
            std::cerr << "[Lua] Erro em OnClientDisconnect: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1); // limpa o erro
        }
    }
    else
    {
        lua_pop(L, 1); // limpa a stack se não for função
    }
}

void LuaBindings::OnGlobalTick()
{
    lua_getglobal(L, "OnGlobalTick"); // pega a função Lua
    if (lua_isfunction(L, -1))
    {
        if (lua_pcall(L, 0, 0, 0) != 0)
        {
            std::cerr << "[Lua] Erro em OnGlobalTick: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1); // limpa a mensagem de erro
        }
    }
    else
    {
        lua_pop(L, 1); // remove se não for função
    }
}


int lua_ClientDisconnect(lua_State* L)
{
    if (!lua_isnumber(L, 1))
    {
        lua_pushstring(L, "ClientDisconnect espera um número como argumento.");
        lua_error(L);
        return 0;
    }

    int cIndex = (int)lua_tointeger(L, 1);
    TCPServer::ClientDisconnect(cIndex);

    return 0; // não retorna nada ao Lua
}

int lua_BufferCreate(lua_State* L)
{
    int id = BufferManager::Create();
    lua_pushinteger(L, id);
    return 1;
}

int lua_BufferFromData(lua_State* L)
{
    size_t len;
    const char* data = luaL_checklstring(L, 1, &len);
    int id = BufferManager::CreateFromBytes(data, len);
    lua_pushinteger(L, id);
    return 1;
}

int lua_BufferDelete(lua_State* L)
{
    int id = luaL_checkinteger(L, 1);
    BufferManager::Delete(id);
    return 0;
}

int lua_BufferReadByte(lua_State* L)
{
    int id = luaL_checkinteger(L, 1);
    Buffer* b = BufferManager::Get(id);
    lua_pushinteger(L, b->ReadByte());
    return 1;
}

int lua_BufferReadWord(lua_State* L)
{
    int id = luaL_checkinteger(L, 1);
    Buffer* b = BufferManager::Get(id);
    lua_pushinteger(L, b->ReadWord());
    return 1;
}

int lua_BufferReadDword(lua_State* L)
{
    int id = luaL_checkinteger(L, 1);
    Buffer* b = BufferManager::Get(id);
    lua_pushinteger(L, b->ReadDword());
    return 1;
}

int lua_BufferReadString(lua_State* L)
{
    int id = luaL_checkinteger(L, 1);
    Buffer* b = BufferManager::Get(id);
    lua_pushstring(L, b->ReadString().c_str());
    return 1;
}

int lua_BufferWriteByte(lua_State* L)
{
    int id = luaL_checkinteger(L, 1);
    int value = luaL_checkinteger(L, 2);
    Buffer* b = BufferManager::Get(id);
    b->WriteByte((unsigned char)value);
    return 0;
}

int lua_BufferWriteWord(lua_State* L)
{
    int id = luaL_checkinteger(L, 1);
    int value = luaL_checkinteger(L, 2);
    Buffer* b = BufferManager::Get(id);
    b->WriteWord((unsigned short)value);
    return 0;
}

int lua_BufferWriteDword(lua_State* L)
{
    int id = luaL_checkinteger(L, 1);
    int value = luaL_checkinteger(L, 2);
    Buffer* b = BufferManager::Get(id);
    b->WriteDword((unsigned int)value);
    return 0;
}

int lua_BufferWriteString(lua_State* L)
{
    int id = luaL_checkinteger(L, 1);
    const char* str = luaL_checkstring(L, 2);
    Buffer* b = BufferManager::Get(id);
    b->WriteString(str);
    return 0;
}

int lua_BufferGetSize(lua_State* L)
{
    int id = luaL_checkinteger(L, 1);
    Buffer* buf = BufferManager::Get(id);
    if (!buf)
    {
        lua_pushnil(L);
        return 1;
    }

    lua_pushinteger(L, static_cast<lua_Integer>(buf->Size()));
    return 1;
}

int lua_SendBuffer(lua_State* L)
{
    int cIndex = luaL_checkinteger(L, 1);
    int id = luaL_checkinteger(L, 2);

    Buffer* b = BufferManager::Get(id);
    if (!b)
    {
        lua_pushstring(L, "Buffer inválido passado para SendBuffer");
        lua_error(L);
        return 0;
    }

    TCPServer::SendRaw(cIndex, b->Data(), b->Size());

    // 🔒 DELETA O BUFFER AQUI, APÓS O ENVIO
    BufferManager::Delete(id);

    return 0;
}

int lua_BufferSetWord(lua_State* L)
{
    int id = luaL_checkinteger(L, 1);
    int offset = luaL_checkinteger(L, 2);
    unsigned short value = static_cast<unsigned short>(luaL_checkinteger(L, 3));

    Buffer* buf = BufferManager::Get(id);
    if (!buf)
    {
        lua_pushboolean(L, 0);
        return 1;
    }

    if (offset + 1 >= buf->Size())
    {
        lua_pushboolean(L, 0); // offset inválido
        return 1;
    }

    // Escreve o valor manualmente no vetor de dados
    buf->data[offset] = value & 0xFF;
    buf->data[offset + 1] = (value >> 8) & 0xFF;

    lua_pushboolean(L, 1);
    return 1;
}

int lua_LogText(lua_State* L)
{
    const char* msg = luaL_checkstring(L, 1);
    std::cout << "[Lua] " << msg << std::endl;
    return 0;
}

int lua_ReloadLua(lua_State* L)
{
    std::cout << "[Lua] Reload Success!" << std::endl;
    LuaBindings::Reload();
    return 0;
}


void LuaBindings::RegisterLuaFunctions(lua_State* L)
{
    lua_register(L, "ClientDisconnect", lua_ClientDisconnect);
    lua_register(L, "BufferGetSize", lua_BufferGetSize);
    lua_register(L, "BufferCreate", lua_BufferCreate);
    lua_register(L, "BufferFromData", lua_BufferFromData);
    lua_register(L, "BufferDelete", lua_BufferDelete);
    lua_register(L, "BufferSetWord", lua_BufferSetWord);
    lua_register(L, "BufferReadByte", lua_BufferReadByte);
    lua_register(L, "BufferReadWord", lua_BufferReadWord);
    lua_register(L, "BufferReadDword", lua_BufferReadDword);
    lua_register(L, "BufferReadString", lua_BufferReadString);

    lua_register(L, "BufferWriteByte", lua_BufferWriteByte);
    lua_register(L, "BufferWriteWord", lua_BufferWriteWord);
    lua_register(L, "BufferWriteDword", lua_BufferWriteDword);
    lua_register(L, "BufferWriteString", lua_BufferWriteString);
    lua_register(L, "ReloadLua", lua_ReloadLua);

    lua_register(L, "SendBuffer", lua_SendBuffer);
    lua_register(L, "LogText", lua_LogText);

    lua_register(L, "DSQuery", lua_DSQuery);
}


