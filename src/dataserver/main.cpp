#include "dataserver.h"
#include "luabidings.h"
#include <thread>
#include <windows.h>

int main()
{
    LuaBindings::Init(); // Inicia o Lua
    DataServer::Start(); // Inicia as threads e servidor UDP

    while (true)
        Sleep(1000); // Mantém ativo

    return 0;
}
