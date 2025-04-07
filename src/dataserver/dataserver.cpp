#include "dataserver.h"
#include "luabidings.h"
#include "udpserver.h"
#include <thread>
#include <iostream>

bool DataServer::queryBusy = false;
std::queue<QueryData> DataServer::queryQueue;
std::mutex DataServer::queueMutex;

void DataServer::Start()
{
    UDPServer::Init(); // Inicializa socket UDP

    std::thread(AcceptThread).detach();
    std::thread(ReceivePacketThread).detach();
    std::thread(QueryProcessingThread).detach();
}

void DataServer::AcceptThread()
{
    std::cout << "[DataServer] Aguardando conexão UDP do GameServer..." << std::endl;
}

void DataServer::ReceivePacketThread()
{
    while (true)
    {
        int size;
        std::string query, label, param;

        if (UDPServer::ReceiveQuery(size, query, label, param))
        {
            QueryData q{ size, query, label, param };
            AddQueryToQueue(q);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void DataServer::QueryProcessingThread()
{
    while (true)
    {
        if (!queryBusy)
        {
            QueryData nextQuery;
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                if (!queryQueue.empty())
                {
                    
                    nextQuery = queryQueue.front();
                    queryQueue.pop();
                    queryBusy = true;
                }
            }

            if (queryBusy)
            {
                LuaBindings::OnDsQuery(nextQuery.size, nextQuery.query, nextQuery.label, nextQuery.param);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void DataServer::AddQueryToQueue(const QueryData& query)
{
    std::lock_guard<std::mutex> lock(queueMutex);
    queryQueue.push(query);
}

void DataServer::MarkQueryCompleted()
{
    queryBusy = false;
}
