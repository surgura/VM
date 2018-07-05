#include "DataWriter.hpp"
#include "IncrementalWriter.hpp"

#include <iostream>
#include <vector>
#include <thread>
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

static constexpr u64 IO_PRINTC_DATA = 3000;
static constexpr u64 IO_PRINTC_ENABLE = 3001;

class PeripheralConsole
{
    DataWriter memory;
    std::thread runner;
    bool run = false;
public:
    PeripheralConsole(u8* memory) :
        memory(memory)
    {
        this->memory.Set(IO_PRINTC_ENABLE, (u8)0);
    }

    void Start()
    {
        runner = std::thread([this]() {
            run = true;
            while(run)
            {
                if(memory.GetU8(IO_PRINTC_ENABLE) == 1)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    std::cout << memory.GetU8(IO_PRINTC_DATA);
                    memory.Set(IO_PRINTC_ENABLE, (u8)0);
                }
            }
        });
    }

    void Stop()
    {
        run = false;
        runner.join();
    }
};

void Run(u8* _memory, u64 offset_program, u64 offset_stack, bool showOpcodes)
{
    DataWriter stack(_memory + offset_stack);
    DataWriter memory(_memory);

    u64 pc = offset_program;
    u64 sp = 0;
    while(true)
    {
        //std::cout << "sp: " << sp << std::endl;
        //std::cout << "pc: " << pc << std::endl;
        Opcode opcode = (Opcode)memory.GetU16(pc);
        //std::cout << (u16)opcode << std::endl;
        switch(opcode)
        {
            case Opcode::jmp:
            {
                if(showOpcodes)
                    std::cout << "jmp";
                u64 addr = memory.GetU64(pc+opcode_size);
                if(showOpcodes)
                    std::cout << ": " << addr << std::endl;
                pc = addr;
            }
            break;
            case Opcode::jmps:
            {
                if(showOpcodes)
                    std::cout << "jmps";
                u64 addr = stack.GetU64(sp-8);
                if(showOpcodes)
                    std::cout << ": " << addr << std::endl;
                sp -= 8;
                pc = addr;
            }
            break;
            case Opcode::jmp_true:
            {
                if(showOpcodes)
                    std::cout << "jmp_true";
                if ((bool)stack.GetU8(sp-1))
                {
                    pc = memory.GetU64(pc+opcode_size);
                    if(showOpcodes)
                        std::cout << ": " << pc << std::endl;
                }
                else
                {
                    pc += opcode_size+8;
                    if(showOpcodes)
                        std::cout << ": false" << std::endl;
                }
                sp -= 1;
            }
            break;
            case Opcode::push_u8:
            {
                if(showOpcodes)
                    std::cout << "push_u8" << std::endl;
                stack.Set(sp, memory.GetU8(pc+opcode_size));
                sp += 1;
                pc += opcode_size+1;
            }
            break;
            case Opcode::push_u64:
            {
                if(showOpcodes)
                    std::cout << "push_u64" << std::endl;
                stack.Set(sp, memory.GetU64(pc+opcode_size));
                sp += 8;
                pc += opcode_size+8;
            }
            break;
            case Opcode::cpl_u8:
            {
                if(showOpcodes)
                    std::cout << "cpl_u8" << std::endl;
                stack.Set(sp, stack.GetU8(sp-memory.GetU64(pc+opcode_size)));
                sp += 1;
                pc += opcode_size+8;
            }
            break;
            case Opcode::cpg_u8:
            {
                if(showOpcodes)
                    std::cout << "cpg_u8" << std::endl;
                stack.Set(sp, memory.GetU8(memory.GetU64(pc+opcode_size)));
                sp += 1;
                pc += opcode_size+8;
            }
            break;
            case Opcode::cmp_u8:
            {
                if(showOpcodes)
                    std::cout << "cmp_u8" << std::endl;
                stack.Set(sp-2, (u8)(stack.GetU8(sp-1) == stack.GetU8(sp-2)));
                sp += -1;
                pc += opcode_size;
            }
            break;
            case Opcode::pop_u8:
            {
                if(showOpcodes)
                    std::cout << "pop_u8" << std::endl;
                sp += -1;
                pc += opcode_size;
            }
            break;
            case Opcode::spd:
            {
                if(showOpcodes)
                    std::cout << "spd" << std::endl;
                sp -= memory.GetU64(pc+opcode_size);
                pc += opcode_size+8;
            }
            break;
            case Opcode::spi:
            {
                if(showOpcodes)
                    std::cout << "spi" << std::endl;
                sp += memory.GetU64(pc+opcode_size);
                pc += opcode_size+8;
            }
            break;
            case Opcode::set_u8:
                if(showOpcodes)
                    std::cout << "set_u8" << std::endl;
                memory.Set(memory.GetU64(pc+opcode_size), stack.GetU8(sp-1));
                sp += -1;
                pc += opcode_size+8;
            break;
            case Opcode::halt:
            {
                std::cout << "halt" << std::endl;
                std::cout << "sp: " << sp << std::endl;
                
                return;
            }
            break;
            default:
            {
                std::cout << "UNKNOWN OPCODE " << (u32)opcode << std::endl;
            }
        }
    }
}

void LoadBin(u8* memory, std::string const& filename, u64 offset)
{
    std::ifstream file(filename, std::ifstream::binary);
    std::vector<u8> bin{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    std::copy(bin.begin(), bin.end(), memory+offset);
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cout << "Usage: " << std::experimental::filesystem::path(argv[0]).stem() << " binary libdir" << std::endl;
        return 1;
    }

    static constexpr u64 offset_program = 0; // must be zero because no PIE
    static constexpr u64 offset_stack = 1000;
    static constexpr u64 offset_console = 2000;
    static constexpr u64 offset_console_printc = offset_console+0;
    static constexpr u64 offset_console_printcstr = offset_console+100;

    std::vector<u8> memory;
    memory.reserve(4000);

    LoadBin(memory.data(), argv[1], offset_program);
    
    LoadBin(memory.data(), std::string(argv[2])+"/console/printc.bin", offset_console_printc);
    LoadBin(memory.data(), std::string(argv[2])+"/console/printcstr.bin", offset_console_printcstr);


    PeripheralConsole perConsole(memory.data());
    perConsole.Start();

    bool showOpcodes = false;
    if (argc == 4)
        showOpcodes = true;

    Run(memory.data(), offset_program, offset_stack, showOpcodes);
    perConsole.Stop();

    return 0;
}