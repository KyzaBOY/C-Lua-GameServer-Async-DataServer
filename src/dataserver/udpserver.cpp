#include "udpserver.h"
#include <winsock2.h>
#include <iostream>

static SOCKET udpSocket;
static sockaddr_in gameServerAddr;

void UDPServer::Init()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(55555);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(udpSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    u_long mode = 1; // 1 = non-blocking
    ioctlsocket(udpSocket, FIONBIO, &mode);
    std::cout << "[UDP] Socket iniciado na porta 55555" << std::endl;
}

bool UDPServer::ReceiveQuery(int& size, std::string& query, std::string& label, std::string& param)
{
    
    char buffer[2048];
    sockaddr_in from;
    int fromLen = sizeof(from);
    int recvLen = recvfrom(udpSocket, buffer, sizeof(buffer), 0, (sockaddr*)&from, &fromLen);

    if (recvLen <= 5) return false;

    gameServerAddr = from;

    int offset = 1; // pula o head
    uint8_t labelLen = buffer[offset++];
    uint8_t paramLen = buffer[offset++];

    uint16_t queryLen = (uint8_t)buffer[offset++];
    queryLen |= ((uint8_t)buffer[offset++] << 8);

    if (recvLen < offset + labelLen + paramLen + queryLen)
        return false;

    label = std::string(buffer + offset, labelLen); offset += labelLen;
    param = std::string(buffer + offset, paramLen); offset += paramLen;
    query = std::string(buffer + offset, queryLen); offset += queryLen;

    size = queryLen;
    return true;
}

void UDPServer::SendResult(const std::string& label, const std::string& param, const std::string& result)
{
    std::string packet;
    packet.push_back(0xFF); // Identificador

    std::string fixedLabel = label;
    std::string fixedParam = param;

    fixedLabel.resize(10, ' ');
    fixedParam.resize(10, ' ');

    packet.append(fixedLabel);
    packet.append(fixedParam);
    packet.append(result); // Result é string pura, sem limitação

    sendto(udpSocket, packet.data(), packet.size(), 0, (sockaddr*)&gameServerAddr, sizeof(gameServerAddr));
}

