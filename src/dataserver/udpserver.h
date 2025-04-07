#pragma once
#include <string>

class UDPServer
{
public:
    static void Init();
    static bool ReceiveQuery(int& size, std::string& query, std::string& label, std::string& param);
    static void SendResult(const std::string& label, const std::string& param, const std::string& result);
};
