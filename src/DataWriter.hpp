#pragma once

#include "Types.hpp"
#include <string>

class DataWriter
{
    u8* array;
public:
    DataWriter(u8* array) :
        array(array)
    {}

    void Set(u64 offset, u8 value)
    {
        array[offset] = value;
    }

    void Set(u64 offset, u16 value)
    {
        // fuck endianness
        array[offset] = ((u8*)(&value))[0];
        array[offset+1] = ((u8*)(&value))[1];
    }

    void Set(u64 offset, u64 value)
    {
        // fuck endianness
        array[offset] = ((u8*)(&value))[0];
        array[offset+1] = ((u8*)(&value))[1];
        array[offset+2] = ((u8*)(&value))[2];
        array[offset+3] = ((u8*)(&value))[3];
        array[offset+4] = ((u8*)(&value))[4];
        array[offset+5] = ((u8*)(&value))[5];
        array[offset+6] = ((u8*)(&value))[6];
        array[offset+7] = ((u8*)(&value))[7];
    }

    u8 GetU8(u64 offset)
    {
        return array[offset];
    }

    u16 GetU16(u64 offset)
    {
        u16 value;
        ((u8*)(&value))[0] = array[offset];
        ((u8*)(&value))[1] = array[offset+1];
        return value;
    }

    u64 GetU64(u64 offset)
    {
        u64 value;
        ((u8*)(&value))[0] = array[offset];
        ((u8*)(&value))[1] = array[offset+1];
        ((u8*)(&value))[2] = array[offset+2];
        ((u8*)(&value))[3] = array[offset+3];
        ((u8*)(&value))[4] = array[offset+4];
        ((u8*)(&value))[5] = array[offset+5];
        ((u8*)(&value))[6] = array[offset+6];
        ((u8*)(&value))[7] = array[offset+7];
        return value;   
    }
};