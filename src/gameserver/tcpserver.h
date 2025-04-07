#pragma once

class TCPServer
{
public:
    static void Start();
    static void ClientDisconnect(int cIndex);
    static void SendRaw(int cIndex, const char* data, int size); // << NOVA FUNÇÃO
    static void ConnectionWatchdogThread(); // ✅ Precisa ser 'static'
};
