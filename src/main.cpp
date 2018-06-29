#include "DataWriter.hpp"
#include "IncrementalWriter.hpp"

#include <iostream>
#include <vector>

static constexpr u8 opcode_size = 2;

enum class Opcode
{
    // args | descr
    jmp = 0, // addr | jump to addr
    jmp_stack, // - | jump to addr on stack
    sp_inc, // offset | increment sp
    sp_dec, // offset | decrease stackpointer
    push_u64, // u64
    halt, // stops machine
    print_u64 = 1000 // u64 | temp test opcode. print u64 on stack
};

void Run(u8* _program, u8* _stack)
{
    static constexpr bool SHOW_OPCODE_NAMES = false;

    DataWriter program(_program);
    DataWriter stack(_stack);

    u64 pc = 0;
    u64 sp = 0;
    while(true)
    {
        //std::cout << "sp: " << sp << std::endl;
        //std::cout << "pc: " << pc << std::endl;
        Opcode opcode = (Opcode)program.GetU16(pc);
        //std::cout << (u16)opcode << std::endl;
        switch(opcode)
        {
            case Opcode::jmp:
            {
                if constexpr(SHOW_OPCODE_NAMES)
                    std::cout << "jmp";
                u64 addr = program.GetU64(pc+opcode_size);
                if constexpr(SHOW_OPCODE_NAMES)
                    std::cout << ": " << addr << std::endl;
                pc = addr;
            }
            break;
            case Opcode::jmp_stack:
            {
                if constexpr(SHOW_OPCODE_NAMES)
                    std::cout << "jmp_stack";
                u64 addr = stack.GetU64(sp-8);
                if constexpr(SHOW_OPCODE_NAMES)
                    std::cout << ": " << addr << std::endl;
                sp -= 8;
                pc = addr;
            }
            break;
            case Opcode::print_u64:
            {
                if constexpr(SHOW_OPCODE_NAMES)
                    std::cout << "print_u64" << std::endl;
                std::cout << stack.GetU64(sp-8) << std::endl;
                pc += opcode_size;
            }
            break;
            case Opcode::push_u64:
            {
                if constexpr(SHOW_OPCODE_NAMES)
                    std::cout << "push_u64" << std::endl;
                stack.Set(sp, program.GetU64(pc+opcode_size));
                sp += 8;
                pc += opcode_size+8;
            }
            break;
            case Opcode::sp_dec:
            {
                if constexpr(SHOW_OPCODE_NAMES)
                    std::cout << "sp_dec" << std::endl;
                sp -= program.GetU64(pc+opcode_size);
                pc += opcode_size+8;
            }
            break;
            case Opcode::sp_inc:
            {
                if constexpr(SHOW_OPCODE_NAMES)
                    std::cout << "sp_inc" << std::endl;
                sp += program.GetU64(pc+opcode_size);
                pc += opcode_size+8;
            }
            break;
            case Opcode::halt:
            {
                if constexpr(SHOW_OPCODE_NAMES)
                    std::cout << "halt" << std::endl;
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

int main()
{
    static constexpr u64 offset_program = 0;
    static constexpr u64 offset_stack = 1000;
    static constexpr u64 offset_os = 2000;

    std::vector<u8> memory;
    memory.reserve(3000);

    IncrementalWriter program(memory.data());
    IncrementalWriter os(memory.data()+offset_os);

    os.Set((u16)Opcode::print_u64);
    os.Set((u16)Opcode::sp_dec);
    os.Set((u64)8);
    os.Set((u16)Opcode::jmp_stack);

    static constexpr u64 ADDR_PRINT_U64 = offset_os + 0;

    program.Set((u16)Opcode::push_u64);
    program.Set((u64)(opcode_size+8+opcode_size+8+opcode_size+8)); // address to instruction after print
    program.Set((u16)Opcode::push_u64);
    program.Set((u64)1234); // u64 to print
    program.Set((u16)Opcode::jmp);
    program.Set((u64)ADDR_PRINT_U64);
    program.Set((u64)Opcode::halt);
    //program.Set((u16)Opcode::jmp); // jump
    //program.Set((u64)0); // addr to begin of program

    Run(memory.data(), memory.data()+offset_stack);

    return 0;
}