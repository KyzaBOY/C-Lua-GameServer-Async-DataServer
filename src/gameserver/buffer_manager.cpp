#include "buffer_manager.h"

std::unordered_map<int, Buffer*> BufferManager::buffers;
std::mutex BufferManager::bufferMutex;
int BufferManager::nextId = 1;

int BufferManager::Create()
{
    std::lock_guard<std::mutex> lock(bufferMutex);
    int id = nextId++;
    buffers[id] = new Buffer();
    return id;
}

int BufferManager::CreateFromBytes(const char* bytes, size_t len)
{
    std::lock_guard<std::mutex> lock(bufferMutex);
    int id = nextId++;

    Buffer* buf = new Buffer();
    for (size_t i = 0; i < len; ++i)
        buf->WriteByte(static_cast<unsigned char>(bytes[i]));

    buffers[id] = buf;
    return id;
}

void BufferManager::Delete(int id)
{
    std::lock_guard<std::mutex> lock(bufferMutex);
    auto it = buffers.find(id);
    if (it != buffers.end())
    {
        delete it->second;
        buffers.erase(it);
    }
}

Buffer* BufferManager::Get(int id)
{
    std::lock_guard<std::mutex> lock(bufferMutex);
    auto it = buffers.find(id);
    if (it != buffers.end())
        return it->second;
    return nullptr;
}

bool BufferManager::Exists(int id)
{
    std::lock_guard<std::mutex> lock(bufferMutex);
    return buffers.count(id) > 0;
}
