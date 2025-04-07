#pragma once
#include <string>
#include <queue>
#include <mutex>

struct QueryData
{
    int size;
    std::string query;
    std::string label;
    std::string param;
};

class DataServer
{
public:
    static void Start();
    static void AddQueryToQueue(const QueryData& query);
    static void MarkQueryCompleted();

private:
    static void AcceptThread();
    static void ReceivePacketThread();
    static void QueryProcessingThread();

    static bool queryBusy;
    static std::queue<QueryData> queryQueue;
    static std::mutex queueMutex;
};
