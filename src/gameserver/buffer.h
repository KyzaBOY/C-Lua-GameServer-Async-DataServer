#pragma once
#include <vector>
#include <string>

class Buffer
{
public:
    Buffer();

    void WriteByte(unsigned char value);
    void WriteWord(unsigned short value);
    void WriteDword(unsigned int value);
    void WriteString(const std::string& value);

    unsigned char ReadByte();
    unsigned short ReadWord();
    unsigned int ReadDword();
    std::string ReadString();

    void Seek(size_t position);
    size_t Tell() const;
    size_t Size() const;

    const char* Data() const;

    std::vector<unsigned char> data;

private:
    
    size_t cursor;
};
