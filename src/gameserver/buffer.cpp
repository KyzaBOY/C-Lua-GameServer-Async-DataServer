#include "buffer.h"
#include <cstring>

Buffer::Buffer()
    : cursor(0)
{
}

void Buffer::WriteByte(unsigned char value)
{
    data.push_back(value);
}

void Buffer::WriteWord(unsigned short value)
{
    data.push_back(value & 0xFF);
    data.push_back((value >> 8) & 0xFF);
}

void Buffer::WriteDword(unsigned int value)
{
    for (int i = 0; i < 4; ++i)
        data.push_back((value >> (i * 8)) & 0xFF);
}

void Buffer::WriteString(const std::string& value)
{
    WriteWord(static_cast<unsigned short>(value.size()));
    data.insert(data.end(), value.begin(), value.end());
}

unsigned char Buffer::ReadByte()
{
    return data[cursor++];
}

unsigned short Buffer::ReadWord()
{
    unsigned short val = data[cursor] | (data[cursor + 1] << 8);
    cursor += 2;
    return val;
}

unsigned int Buffer::ReadDword()
{
    unsigned int val = 0;
    for (int i = 0; i < 4; ++i)
        val |= (data[cursor++] << (i * 8));
    return val;
}

std::string Buffer::ReadString()
{
    unsigned short len = ReadWord();
    std::string result(data.begin() + cursor, data.begin() + cursor + len);
    cursor += len;
    return result;
}

void Buffer::Seek(size_t position)
{
    cursor = position;
}

size_t Buffer::Tell() const
{
    return cursor;
}

size_t Buffer::Size() const
{
    return data.size();
}

const char* Buffer::Data() const
{
    // Retorna ponteiro válido mesmo se estiver vazio
    static const char dummy = 0;
    if (data.empty()) return &dummy;
    return reinterpret_cast<const char*>(data.data());
}
