#pragma once
#include "buffer_manager.h"
#include "buffer.h"
// Biblioteca padrão Lua
extern "C" {
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
}

namespace LuaBindings
{
    // Inicializa o estado do Lua (lua_State*) e carrega o script principal
    void Init();

    void Reload();

    // Chamada quando um cliente se conecta
    void OnClientConnect(int cIndex);
    void OnClientDisconnect(int cIndex);
    // Chamada quando um pacote é recebido
    void OnPacketRecv(int cIndex, int hCode, int sCode, const char* data, int dataLen);
    void OnGlobalTick(); // 👈 agora fica igual aos outros

    void RegisterLuaFunctions(lua_State* L);
    void OnSQLAsyncResult(const std::string& label, const std::string& param, const std::string& result);
    // (Opcional futuro) Você poderá adicionar aqui funções para expor libs de buffer, funções utilitárias, etc.
    // Ex: void RegisterBufferFunctions(lua_State* L);
}
