#include <windows.h>
#include "gameserver.h"
#include "luabidings.h"
#include <synchapi.h>
#include <iostream>
#include <string>
#include "tcpserver.h"
#include <thread>
#include "udpserver.h"


#pragma message("Arquitetura detectada:")
#ifdef _M_IX86
#pragma message("Compilando para: x86")
#endif
#ifdef _M_X64
#pragma message("Compilando para: x64")
#endif


void CommandConsoleThread()
{
    std::string input;
    while (true)
    {
        std::getline(std::cin, input);

        if (input == "reload")
        {
            std::cout << "[Console] Recarregando scripts Lua..." << std::endl;
            LuaBindings::Reload();
        }
    }
}



int main()
{
    LuaBindings::Init();
    GameServer::Start();
    std::thread(TCPServer::ConnectionWatchdogThread).detach();
    std::thread(CommandConsoleThread).detach();
    UDPServer::Init();
    UDPServer::StartReceiveThread();
    while (true) {
        Sleep(1000);
    }

    return 0;
}



