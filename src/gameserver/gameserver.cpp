#include "gameserver.h"
#include "tcpserver.h"
#include <iostream>

void GameServer::Start()
{
    std::cout << "[GameServer] Iniciando servidor..." << std::endl;
    TCPServer::Start();
}
