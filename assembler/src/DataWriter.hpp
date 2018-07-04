#pragma once

#include "Types.hpp"
#include <string>
#include <vector>

class DataWriter
{
    std::vector<u8>& data;
public:
    DataWriter(std::vector<u8>& data) :
        data(data)
    {}

    void Add(u8 value)
    {
        data.push_back(value);
    }

    void Add(u16 value)
    {
        // fuck endianness
        data.push_back(((u8*)(&value))[0]);
        data.push_back(((u8*)(&value))[1]);
    }

    void Add(u64 value)
    {
        // fuck endianness
        data.push_back(((u8*)(&value))[0]);
        data.push_back(((u8*)(&value))[1]);
        data.push_back(((u8*)(&value))[2]);
        data.push_back(((u8*)(&value))[3]);
        data.push_back(((u8*)(&value))[4]);
        data.push_back(((u8*)(&value))[5]);
        data.push_back(((u8*)(&value))[6]);
        data.push_back(((u8*)(&value))[7]);
    }

    void Add(s64 value)
    {
        // fuck endianness
        data.push_back(((u8*)(&value))[0]);
        data.push_back(((u8*)(&value))[1]);
        data.push_back(((u8*)(&value))[2]);
        data.push_back(((u8*)(&value))[3]);
        data.push_back(((u8*)(&value))[4]);
        data.push_back(((u8*)(&value))[5]);
        data.push_back(((u8*)(&value))[6]);
        data.push_back(((u8*)(&value))[7]);
    }
};