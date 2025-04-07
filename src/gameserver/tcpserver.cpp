#include "tcpserver.h"
#include "luabidings.h"
#include "buffer_manager.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>
#include <iostream>
#include <atomic>
#pragma comment(lib, "ws2_32.lib")
#include <unordered_map>
#include <mutex>

static std::unordered_map<int, SOCKET> clientSockets;
static std::mutex clientSocketsMutex;
static std::atomic<int> clientIndexCounter(0);
static SOCKET listenSocket;

void AcceptThread()
{
    while (true)
    {
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);

        SOCKET clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &clientSize);
        if (clientSocket != INVALID_SOCKET)
        {
            int cIndex = clientIndexCounter.fetch_add(1);

            {
                std::lock_guard<std::mutex> lock(clientSocketsMutex);
                clientSockets[cIndex] = clientSocket;
            }

            std::cout << "[TCPServer] Cliente conectado! cIndex: " << cIndex << std::endl;

            // 🔥 Aqui o mutex já foi liberado — então é seguro chamar Lua
            LuaBindings::OnClientConnect(cIndex);
        }
    }
}

void PacketReceiveThread()
{
    char recvBuffer[2048];

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        std::unordered_map<int, SOCKET> socketsCopy;
        {
            std::lock_guard<std::mutex> lock(clientSocketsMutex);
            socketsCopy = clientSockets;
        }

        for (auto& pair : socketsCopy)
        {
            int cIndex = pair.first;
            SOCKET socket = pair.second;

            u_long bytesAvailable = 0;
            if (ioctlsocket(socket, FIONREAD, &bytesAvailable) != 0)
                continue;

            if (bytesAvailable == 0)
                continue;

            int received = recv(socket, recvBuffer, sizeof(recvBuffer), 0);

            // 🔎 Log sempre que algum dado for recebido, independente do tamanho
            std::cout << "[Packet RAW] cIndex: " << cIndex << " (" << received << " bytes): ";
            for (int i = 0; i < received; ++i) {
                printf("%02X ", static_cast<unsigned char>(recvBuffer[i]));
            }
            std::cout << std::endl;

            if (received <= 0)
            {
                std::cerr << "[TCPServer] Cliente " << cIndex << " desconectado durante leitura." << std::endl;
                TCPServer::ClientDisconnect(cIndex);
                continue;
            }

            int offset = 0;
            while (received - offset >= 4)
            {
                unsigned char head = recvBuffer[offset];
                unsigned char sub = recvBuffer[offset + 1];
                unsigned char sizeL = recvBuffer[offset + 2];
                unsigned char sizeH = recvBuffer[offset + 3];
                int fullSize = sizeL | (sizeH << 8);

                if (fullSize < 4 || received - offset < fullSize)
                {
                    std::cerr << "[TCPServer] ⚠️ Pacote incompleto ou inválido de cIndex " << cIndex << ". fullSize=" << fullSize << std::endl;
                    break;
                }

                const char* dataStart = recvBuffer + offset + 4;
                int dataLen = fullSize - 4;

                LuaBindings::OnPacketRecv(cIndex, head, sub, dataStart, dataLen);

                offset += fullSize;
            }

            // Se sobrar bytes (< 4), avisa também
            if (received < 4)
            {
                std::cerr << "[TCPServer] ⚠️ Pacote menor que 4 bytes de cIndex " << cIndex << ". Ignorado." << std::endl;
            }
        }
    }
}

void TCPServer::Start()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(44405);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(listenSocket, SOMAXCONN);

    std::cout << "[TCPServer] Escutando na porta 44405..." << std::endl;

    std::thread(AcceptThread).detach();
    std::thread(PacketReceiveThread).detach(); // <- adiciona isso
}

void TCPServer::ClientDisconnect(int cIndex)
{
    std::lock_guard<std::mutex> lock(clientSocketsMutex);

    auto it = clientSockets.find(cIndex);
    if (it != clientSockets.end())
    {
        SOCKET s = it->second;
        shutdown(s, SD_BOTH); // Encerra envio e recebimento
        closesocket(s);       // Fecha o socket

        clientSockets.erase(it);

        std::cout << "[TCPServer] Cliente desconectado! cIndex: " << cIndex << std::endl;
        LuaBindings::OnClientDisconnect(cIndex);
    }
    else
    {
        std::cerr << "[TCPServer] Tentativa de desconectar cliente inválido (cIndex: " << cIndex << ")" << std::endl;
    }
}

void TCPServer::SendRaw(int cIndex, const char* data, int size)
{
    if (!data || size <= 0) {
        std::cerr << "[TCPServer] ❌ Tentativa de envio com buffer nulo ou tamanho inválido (size = " << size << ")." << std::endl;
        return;
    }
    // Tenta adquirir o mutex
    if (!clientSocketsMutex.try_lock()) {
        std::cerr << "[TCPServer] ⚠️ Mutex já está travada. Ignorando envio para cIndex " << cIndex << std::endl;
        return;
    }
    // Garantimos unlock após esta parte
    std::lock_guard<std::mutex> unlockLater(clientSocketsMutex, std::adopt_lock);

    auto it = clientSockets.find(cIndex);
    if (it == clientSockets.end()) {
        std::cerr << "[TCPServer] ❌ Tentativa de envio para cIndex inválido: " << cIndex << std::endl;
        return;
    }

    SOCKET clientSocket = it->second;

    // Print os primeiros bytes do buffer como hexa
    std::cout << "[TCPServer] Dados (hex): ";
    for (int i = 0; i < (size < 16 ? size : 16); ++i) {
        printf("%02X ", static_cast<unsigned char>(data[i]));
    }
    if (size > 16) std::cout << "...";
    std::cout << std::endl;

    int sent = send(clientSocket, data, size, 0);
    if (sent == SOCKET_ERROR)
    {
        std::cerr << "[TCPServer] ❌ Erro ao enviar para cIndex " << cIndex << ": " << WSAGetLastError() << std::endl;
        ClientDisconnect(cIndex);
    }
    else
    {
        std::cout << "[TCPServer] Packet Sent!" << std::endl;
    }
}

void TCPServer::ConnectionWatchdogThread()
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1)); // A cada 1 segundo

        std::unordered_map<int, SOCKET> socketsCopy;
        {
            std::lock_guard<std::mutex> lock(clientSocketsMutex);
            socketsCopy = clientSockets; // Cópia local
        }

        for (const auto& pair : socketsCopy)
        {
            int cIndex = pair.first;
            SOCKET sock = pair.second;

            // Validação (exemplo básico): envia ping ou verifica tempo inativo
            u_long check = 0;
            int result = ioctlsocket(sock, FIONREAD, &check);

            if (result != 0 || check == SOCKET_ERROR)
            {
                std::cerr << "[Watchdog] Erro ao verificar cliente " << cIndex << ". Desconectando..." << std::endl;
                ClientDisconnect(cIndex); // ⚠️ Esta função já trata o mutex corretamente
                continue;
            }

            // (Opcional) lógica de timeout personalizada...
        }
        // Tick global a cada 1 segundo
        LuaBindings::OnGlobalTick();
    }
}




