#pragma once
#include <string>

namespace UDPServer
{
    void Init();
    void SendQuery(const std::string& query, const std::string& label, const std::string& param);
    void StartReceiveThread();
}
