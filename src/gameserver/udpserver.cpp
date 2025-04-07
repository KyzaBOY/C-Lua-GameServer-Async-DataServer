#include "udpserver.h"
#include <winsock2.h>
#include <thread>
#include <iostream>
#include "luabidings.h"
#include <ws2tcpip.h> // para inet_pton

static SOCKET udpSocket;
static sockaddr_in dsAddr;

void UDPServer::Init()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in localAddr{};
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(55556); // porta LOCAL
    localAddr.sin_addr.s_addr = INADDR_ANY;

    bind(udpSocket, (sockaddr*)&localAddr, sizeof(localAddr));

    dsAddr.sin_family = AF_INET;
    dsAddr.sin_port = htons(55555); // porta do DataServer
    inet_pton(AF_INET, "127.0.0.1", &dsAddr.sin_addr);

}

void UDPServer::SendQuery(const std::string& query, const std::string& label, const std::string& param)
{
    std::string paddedLabel = label;
    paddedLabel.resize(10, ' '); // completa com espaços se menor que 10

    std::string paddedParam = param;
    paddedParam.resize(10, ' '); // completa com espaços se menor que 10

    uint16_t querySize = (uint16_t)query.size();

    std::string packet;
    packet.push_back(0x01); // cabeçalho
    packet.push_back((uint8_t)paddedLabel.size());
    packet.push_back((uint8_t)paddedParam.size());
    packet.push_back(querySize & 0xFF);
    packet.push_back((querySize >> 8) & 0xFF);

    packet.append(paddedLabel);
    packet.append(paddedParam);
    packet.append(query);

    sendto(udpSocket, packet.data(), packet.size(), 0, (sockaddr*)&dsAddr, sizeof(dsAddr));
}


void UDPServer::StartReceiveThread()
{
    std::thread([]() {
        char buffer[4096];
        sockaddr_in from;
        int fromLen = sizeof(from);

        while (true)
        {
            int len = recvfrom(udpSocket, buffer, sizeof(buffer), 0, (sockaddr*)&from, &fromLen);
            if (len >= 22 && buffer[0] == (char)0xFF) // 1 + 10 + 10 + X
            {
                std::string rawLabel(buffer + 1, 10);
                std::string rawParam(buffer + 11, 10);
                std::string rawResult(buffer + 21, len - 21); // <-- O RESULTADO REAL

                // Trimming de \0 ou espaços
                std::string label = rawLabel.substr(0, rawLabel.find_first_of('\0'));
                std::string param = rawParam.substr(0, rawParam.find_first_of('\0'));

                label.erase(label.find_last_not_of(" ") + 1);
                param.erase(param.find_last_not_of(" ") + 1);

                LuaBindings::OnSQLAsyncResult(label, param, rawResult);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        }).detach();
}
