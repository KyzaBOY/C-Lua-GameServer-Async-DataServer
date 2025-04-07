#pragma once
#include "buffer.h"
#include <unordered_map>
#include <mutex>

class BufferManager
{
public:
    static int Create();
    static int CreateFromBytes(const char* bytes, size_t len);
    static void Delete(int id);
    static Buffer* Get(int id);
    static bool Exists(int id);

private:
    static std::unordered_map<int, Buffer*> buffers;
    static std::mutex bufferMutex;
    static int nextId;
};
