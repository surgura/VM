#pragma once

#include "DataWriter.hpp"

class IncrementalWriter
{
    DataWriter writer;
    u64 offset = 0;
public:
    IncrementalWriter(u8* array) :
        writer(array)
    {}

    void Set(u8 value)
    {
        writer.Set(offset, value);
        offset += 1;
    }

    void Set(u16 value)
    {
        writer.Set(offset, value);
        offset += 2;
    }

    void Set(u64 value)
    {
        writer.Set(offset, value);
        offset += 8;
    }
};