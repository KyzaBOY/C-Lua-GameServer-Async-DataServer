#pragma once
#include <string>

class LuaBindings
{
public:
    static void Init();
    static void OnDsQuery(int size, const std::string& query, const std::string& label, const std::string& param);
};
