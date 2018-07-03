#include "DataWriter.hpp"
#include "IncrementalWriter.hpp"

#include <iostream>
#include <vector>
#include <thread>

static constexpr u8 opcode_size = 2;

enum class Opcode
{
    // args | descr
    jmp = 0, // addr | jump to addr
    jmp_stack, // - | jump to addr on stack. consumes addr.
    jmp_true, // address | if byte on stack is 1, jump to address. consumes byte.
    cmp_u8, // - | consumes two bytes from stack. pushes true on stack if equal. else false.
    sp_inc, // offset | increment sp
    sp_dec, // offset | decrease stackpointer
    push_u8,
    push_u64,
    pop_u8,
    set_u8, // offset | set memory to u8 from stack
    cp_u8, // offset | copy relative byte to stack
    cp_abs_u8, // addr | copy absolute byte to stack
    halt, // stops machine
    print_u64 = 1000, // - | temp test opcode. print u64 on stack
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
                //std::cout << (u64)memory.GetU8(IO_PRINTC_ENABLE) << std::endl;
                if(memory.GetU8(IO_PRINTC_ENABLE) == 1)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
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

void Run(u8* _program, u8* _stack, u8* _memory)
{
    static constexpr bool SHOW_OPCODE_NAMES = true;
    
    DataWriter program(_program);
    DataWriter stack(_stack);
    DataWriter memory(_memory);

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
            case Opcode::jmp_true:
            {
                if constexpr(SHOW_OPCODE_NAMES)
                    std::cout << "jmp_true" << std::endl;
                if ((bool)stack.GetU8(sp-1))
                    pc = program.GetU64(pc+opcode_size);
                else
                    pc += opcode_size+8;
                sp -= 1;
            }
            break;
            case Opcode::print_u64:
            {
                if constexpr(SHOW_OPCODE_NAMES)
                    std::cout << "print_u64" << std::endl;
                std::cout << stack.GetU64(sp-8) << std::endl;
                sp -= 8;
                pc += opcode_size;
            }
            break;
            case Opcode::push_u8:
            {
                if constexpr(SHOW_OPCODE_NAMES)
                    std::cout << "push_u8" << std::endl;
                stack.Set(sp, program.GetU8(pc+opcode_size));
                sp += 1;
                pc += opcode_size+1;
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
            case Opcode::cp_u8:
            {
                if constexpr(SHOW_OPCODE_NAMES)
                    std::cout << "cp_u8" << std::endl;
                stack.Set(sp, stack.GetU8(sp-program.GetU64(pc+opcode_size)));
                sp += 1;
                pc += opcode_size+8;
            }
            break;
            case Opcode::cp_abs_u8:
            {
                if constexpr(SHOW_OPCODE_NAMES)
                    std::cout << "cp_abs_u8" << std::endl;
                stack.Set(sp, memory.GetU8(program.GetU64(pc+opcode_size)));
                sp += 1;
                pc += opcode_size+8;
            }
            break;
            case Opcode::cmp_u8:
            {
                if constexpr(SHOW_OPCODE_NAMES)
                    std::cout << "cmp_u8" << std::endl;
                stack.Set(sp-2, (u8)(stack.GetU8(sp-1) == stack.GetU8(sp-2)));
                sp += -1;
                pc += opcode_size;
            }
            break;
            case Opcode::pop_u8:
            {
                if constexpr(SHOW_OPCODE_NAMES)
                    std::cout << "pop_u8" << std::endl;
                sp += -1;
                pc += opcode_size;
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
            case Opcode::set_u8:
                if constexpr(SHOW_OPCODE_NAMES)
                    std::cout << "set_u8" << std::endl;
                memory.Set(program.GetU64(pc+opcode_size), stack.GetU8(sp-1));
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

int main()
{
    static constexpr u64 offset_program = 0;
    static constexpr u64 offset_stack = 1000;
    static constexpr u64 offset_os = 2000;

    std::vector<u8> memory;
    memory.reserve(4000);

    PeripheralConsole perConsole(memory.data());
    perConsole.Start();

    IncrementalWriter program(memory.data());
    IncrementalWriter os(memory.data()+offset_os);

    const u64 ADDR_PRINT_U64 = offset_os + os.Pos();
    os.Set((u16)Opcode::print_u64);
    os.Set((u16)Opcode::jmp_stack);

    // expects stack
    // - return addr
    // - u8
    const u64 ADDR_PRINT_CHAR = offset_os + os.Pos();
    os.Set((u16)Opcode::set_u8);
    os.Set((u64)IO_PRINTC_DATA);
    os.Set((u16)Opcode::push_u8);
    os.Set((u8)1);
    os.Set((u16)Opcode::set_u8);
    os.Set((u64)IO_PRINTC_ENABLE);
    os.Set((u16)Opcode::cp_abs_u8);
    os.Set((u64)IO_PRINTC_ENABLE);
    os.Set((u16)Opcode::jmp_true);
    os.Set((u64)ADDR_PRINT_CHAR+opcode_size*3+8*2+1);
    os.Set((u16)Opcode::jmp_stack);

    /*
    const u64 ADDR_PRINT_CSTR = offset_os + os.Pos();
    os.Set((u16)Opcode::cp_u8);
    os.Set((u64)1);
    os.Set((u16)Opcode::push_u8);
    os.Set((u8)'\0');
    os.Set((u16)Opcode::cmp_u8);
    os.Set((u16)Opcode::jmp_true);
    os.Set((u64)ADDR_PRINT_CSTR+opcode_size+8+opcode_size+1+opcode_size+opcode_size+8+opcode_size+opcode_size+8); // TODO
    os.Set((u16)Opcode::print_char);
    os.Set((u16)Opcode::jmp);
    os.Set((u64)ADDR_PRINT_CSTR);
    os.Set((u16)Opcode::pop_u8);
    os.Set((u16)Opcode::push_u8);
    os.Set((u8)'\n');
    os.Set((u16)Opcode::print_char);
    os.Set((u16)Opcode::jmp_stack);*/

    program.Set((u16)Opcode::push_u64);
    program.Set((u64)(opcode_size+8+opcode_size+8+opcode_size+8)); // address to instruction after print
    program.Set((u16)Opcode::push_u64);
    program.Set((u64)1234); // u64 to print
    program.Set((u16)Opcode::jmp);
    program.Set((u64)ADDR_PRINT_U64);

/*
    const u64 ADDR_PRINT_HW = program.Pos();
    program.Set((u16)Opcode::push_u64);
    program.Set((u64)(ADDR_PRINT_HW+opcode_size+8+14*(opcode_size+1)+opcode_size+8)); // address to instruction after print
    program.Set((u16)Opcode::push_u8);
    program.Set((u8)'\0');
    program.Set((u16)Opcode::push_u8);
    program.Set((u8)'!');
    program.Set((u16)Opcode::push_u8);
    program.Set((u8)'d');
    program.Set((u16)Opcode::push_u8);
    program.Set((u8)'l');
    program.Set((u16)Opcode::push_u8);
    program.Set((u8)'r');
    program.Set((u16)Opcode::push_u8);
    program.Set((u8)'o');
    program.Set((u16)Opcode::push_u8);
    program.Set((u8)'w');
    program.Set((u16)Opcode::push_u8);
    program.Set((u8)' ');
    program.Set((u16)Opcode::push_u8);
    program.Set((u8)'o');
    program.Set((u16)Opcode::push_u8);
    program.Set((u8)'l');
    program.Set((u16)Opcode::push_u8);
    program.Set((u8)'l');
    program.Set((u16)Opcode::push_u8);
    program.Set((u8)'e');
    program.Set((u16)Opcode::push_u8);
    program.Set((u8)'H');
    program.Set((u16)Opcode::push_u8);
    program.Set((u8)'5');
    program.Set((u16)Opcode::jmp);
    program.Set((u64)ADDR_PRINT_CSTR);*/
    
    const u64 ADDR_PRINT_HW = program.Pos();
    program.Set((u16)Opcode::push_u64);
    program.Set((u64)(ADDR_PRINT_HW+opcode_size*3+8+1+8)); // address to instruction after print
    program.Set((u16)Opcode::push_u8);
    program.Set((u8)'X');
    program.Set((u16)Opcode::jmp);
    program.Set((u64)ADDR_PRINT_CHAR);
    program.Set((u64)Opcode::halt);
    //program.Set((u16)Opcode::jmp); // jump
    //program.Set((u64)0); // addr to begin of program

    Run(memory.data(), memory.data()+offset_stack, memory.data());
    perConsole.Stop();

    return 0;
}