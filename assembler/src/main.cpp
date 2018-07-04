#include "DataWriter.hpp"

#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include <fstream>
#include <filesystem>

static constexpr u8 opcode_size = 2;

enum class Opcode
{
    // args | descr
    jmp = 0, // addr | jump to addr
    jmps, // - | jump to addr on stack. consumes addr.
    jmp_true, // address | if byte on stack is 1, jump to address. consumes byte.
    cmp_u8, // - | consumes two bytes from stack. pushes true on stack if equal. else false.
    spi, // offset | increment sp
    spd, // offset | decrease stackpointer
    push_u8, // u8 | push byte on stack
    push_u64,
    pop_u8,
    set_u8, // offset | set memory to u8 from stack
    cpl_u8, // offset | copy relative(local) byte to stack
    cpg_u8, // addr | copy absolute(global) byte to stack
    halt, // stops machine
};

std::tuple<std::string::const_iterator, std::vector<std::string>> ParseRow(std::string::const_iterator rowBegin, std::string::const_iterator end)
{
    auto isWhiteSpace = [](char const& c){ return c == ' ' || c == '\t' || c == '\n'; };
    auto isNotWhiteSpace = [&isWhiteSpace](char const& c){ return !isWhiteSpace(c); };

    std::vector<std::string> words;
    while(true)
    {
        auto wordBegin = std::find_if(rowBegin, end, isNotWhiteSpace);
        auto wordEnd = std::find_if(wordBegin, end, isWhiteSpace);

        std::string word(wordBegin, wordEnd);
        words.push_back(word);

        if (wordEnd == end || *wordEnd == '\n')
        {
            if (wordEnd != end)
                ++wordEnd;
            return { wordEnd, words };
        }

        rowBegin = wordEnd;
    }
}

void ReqArgcount(std::vector<std::string> const& tokens, u64 argcount, std::string const& opcode)
{
    if (tokens.size()-1 != argcount )
    {
        std::string msg = opcode + " requires " + std::to_string(argcount) + " arguments.";
        std::cout << msg << std::endl;
        throw std::runtime_error(msg);
    }
}

u64 ArgHex(std::string const& hex)
{
    return std::stoul(hex, nullptr, 16);
}

std::vector<u8> AssembleOperation(std::vector<std::string> const& tokens)
{
    static constexpr bool showOpcodes = true;

    std::vector<u8> result;
    DataWriter writer(result);

    std::string const& opcode = tokens[0];
    if (opcode == "jmp")
    {
        if constexpr(showOpcodes)
            std::cout << "Opcode jmp" << std::endl;
        ReqArgcount(tokens, 1, opcode);
        u64 addr = ArgHex(tokens[1]);
        writer.Add((u16)Opcode::jmp);
        writer.Add(addr);
    }
    else if (opcode == "jmps")
    {
        if constexpr(showOpcodes)
            std::cout << "Opcode jmps" << std::endl;
        ReqArgcount(tokens, 0, opcode);
        writer.Add((u16)Opcode::jmps);
    }
    else if (opcode == "jmp_true")
    {
        if constexpr(showOpcodes)
            std::cout << "Opcode jmp_true" << std::endl;
        ReqArgcount(tokens, 1, opcode);
        u64 addr = ArgHex(tokens[1]);
        writer.Add((u16)Opcode::jmp_true);
        writer.Add(addr);
    }
    else if (opcode == "cmp_u8")
    {
        if constexpr(showOpcodes)
            std::cout << "Opcode cmp_u8" << std::endl;
        ReqArgcount(tokens, 0, opcode);
        writer.Add((u16)Opcode::cmp_u8);
    }
    else if (opcode == "spi")
    {
        if constexpr(showOpcodes)
            std::cout << "Opcode spi" << std::endl;
        ReqArgcount(tokens, 1, opcode);
        s64 offset = ArgHex(tokens[1]);
        writer.Add((u16)Opcode::spi);
        writer.Add(offset);
    }
    else if (opcode == "spd")
    {
        if constexpr(showOpcodes)
            std::cout << "Opcode spd" << std::endl;
        ReqArgcount(tokens, 1, opcode);
        s64 offset = ArgHex(tokens[1]);
        writer.Add((u16)Opcode::spd);
        writer.Add(offset);
    }
    else if (opcode == "push_u8")
    {
        if constexpr(showOpcodes)
            std::cout << "Opcode push_u8" << std::endl;
        ReqArgcount(tokens, 1, opcode);
        u8 val = ArgHex(tokens[1]);
        writer.Add((u16)Opcode::push_u8);
        writer.Add(val);
    }
    else if (opcode == "push_u64")
    {
        if constexpr(showOpcodes)
            std::cout << "Opcode push_u64" << std::endl;
        ReqArgcount(tokens, 1, opcode);
        u64 val = ArgHex(tokens[1]);
        writer.Add((u16)Opcode::push_u64);
        writer.Add(val);
    }
    else if (opcode == "pop_u8")
    {
        if constexpr(showOpcodes)
            std::cout << "Opcode pop_u8" << std::endl;
        ReqArgcount(tokens, 0, opcode);
        writer.Add((u16)Opcode::pop_u8);
    }
    else if (opcode == "set_u8")
    {
        if constexpr(showOpcodes)
            std::cout << "Opcode set_u8" << std::endl;
        ReqArgcount(tokens, 1, opcode);
        u64 addr = ArgHex(tokens[1]);
        writer.Add((u16)Opcode::set_u8);
        writer.Add(addr);
    }
    else if (opcode == "cpl_u8")
    {
        if constexpr(showOpcodes)
            std::cout << "Opcode cpl_u8" << std::endl;
        ReqArgcount(tokens, 1, opcode);
        u64 addr = ArgHex(tokens[1]);
        writer.Add((u16)Opcode::cpl_u8);
        writer.Add(addr);
    }
    else if (opcode == "cpg_u8")
    {
        if constexpr(showOpcodes)
            std::cout << "Opcode cpg_u8" << std::endl;
        ReqArgcount(tokens, 1, opcode);
        u64 addr = ArgHex(tokens[1]);
        writer.Add((u16)Opcode::cpg_u8);
        writer.Add(addr);
    }
    else if (opcode == "halt")
    {
        if constexpr(showOpcodes)
            std::cout << "Opcode halt" << std::endl;
        ReqArgcount(tokens, 0, opcode);
        writer.Add((u16)Opcode::halt);
    }
    else
    {
        auto msg = "Unknown opcode: "+opcode;
        std::cout << msg << std::endl;
        throw std::runtime_error(msg);
    }
    return result;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cout << "Usage: " << std::experimental::filesystem::path(argv[0]).stem() << " outfile infile" << std::endl;
    }

    std::vector<u8> bin;

    std::ifstream file(argv[2]);
    std::string const program{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    file.close();

    auto progPos = program.begin();
    while(progPos != program.end())
    {
        auto row = ParseRow(program.begin(), program.end());
        progPos = std::get<0>(row);
        if (std::get<1>(row).size() != 0)
        {
            std::vector<u8> opBin = AssembleOperation(std::get<1>(row));
            bin.insert(bin.end(), opBin.begin(), opBin.end());
        }
    }

    std::ofstream outfile (argv[1],std::ofstream::binary);
    outfile.write((char const*)bin.data(), bin.size());

    return 0;
}