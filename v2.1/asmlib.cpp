#include"writelist.h"

//class Code
//{
  //public:
  //  Code();
  //  ~Code();

  //  Code(const Code &) = delete;
  //  Code &operator=(const Code &) = delete;

    void asm_init();

    void asm_add_byte(unsigned char);
    void asm_add_word(unsigned short);
    void asm_add_dword(unsigned int);

    void asm_add_list(std::initializer_list<unsigned char>);
    template <typename... Args>
    void asm_add_list(Args...);

    void asm_push_byte(unsigned char);
    void asm_push_dword(unsigned int);

    void asm_mov_exx(Reg, unsigned int);
    void asm_mov_exx_dword_ptr(Reg, unsigned int);
    void asm_mov_exx_dword_ptr_exx_add(Reg, unsigned int);

    void asm_push_exx(Reg);
    void asm_pop_exx(Reg);
    void asm_mov_exx_exx(Reg, Reg);

    void asm_call(unsigned int);

    void asm_ret();

    void asm_code_inject(HANDLE);

  //protected:
    unsigned char *code;
    unsigned int length;
    std::vector<unsigned int> calls_pos;
//};

template <typename... Args>
void asm_add_list(Args... value)
{
    asm_add_list({static_cast<unsigned char>(value)...});
}

void asm_init()
{
    length = 0;
    calls_pos.clear();
}

void asm_add_byte(unsigned char value)
{
    code[length] = value;
    length += 1;
}

void asm_add_word(unsigned short value)
{
    (unsigned short &)code[length] = value;
    length += 2;
}

void asm_add_dword(unsigned int value)
{
    (unsigned int &)code[length] = value;
    length += 4;
}

void asm_add_list(std::initializer_list<unsigned char> value)
{
    for (auto it = value.begin(); it != value.end(); it++)
        asm_add_byte(*it);
}

void asm_push_byte(unsigned char value)
{
    asm_add_byte(0x6a);
    asm_add_byte(value);
}

void asm_push_dword(unsigned int value)
{
    asm_add_byte(0x68);
    asm_add_dword(value);
}

void asm_mov_exx(Reg reg, unsigned int value)
{
    asm_add_byte(0xb8 + static_cast<unsigned int>(reg));
    asm_add_dword(value);
}

void asm_mov_exx_dword_ptr(Reg reg, unsigned int value)
{
    asm_add_byte(0x8b);
    asm_add_byte(0x05 + static_cast<unsigned int>(reg) * 8);
    asm_add_dword(value);
}

void asm_mov_exx_dword_ptr_exx_add(Reg reg, unsigned int value)
{
    asm_add_byte(0x8b);
    asm_add_byte(0x80 + static_cast<unsigned int>(reg) * (8 + 1));
    if (reg == Reg::ESP)
        asm_add_byte(0x24);
    asm_add_dword(value);
}

void asm_push_exx(Reg reg)
{
    asm_add_byte(0x50 + static_cast<unsigned int>(reg));
}

void asm_pop_exx(Reg reg)
{
    asm_add_byte(0x58 + static_cast<unsigned int>(reg));
}

void asm_mov_exx_exx(Reg reg_to, Reg reg_from)
{
    asm_add_byte(0x8b);
    asm_add_byte(0xc0 + static_cast<unsigned int>(reg_to) * 8 + static_cast<unsigned int>(reg_from));
}

void asm_call(unsigned int addr)
{
    asm_add_byte(0xe8);
    calls_pos.push_back(length);
    asm_add_dword(addr);
}

void asm_ret()
{
    asm_add_byte(0xc3);
}

void asm_code_inject(HANDLE handle)
{
    LPVOID addr = VirtualAllocEx(handle, nullptr, length, //
                                 MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (addr == nullptr)
        return;

    for (size_t i = 0; i < calls_pos.size(); i++)
    {
        unsigned int pos = calls_pos[i];
        int &call_addr = (int &)(code[pos]);
        call_addr = call_addr - ((int)addr + pos + 4);
    }

    SIZE_T write_size = 0;
    BOOL ret = WriteProcessMemory(handle, addr, code, length, &write_size);
    if (ret == 0 || write_size != length)
    {
        VirtualFreeEx(handle, addr, 0, MEM_RELEASE);
        return;
    }

    HANDLE thread = CreateRemoteThread //
        (handle, nullptr, 0, LPTHREAD_START_ROUTINE(addr), nullptr, 0, nullptr);
    if (thread == nullptr)
    {
        VirtualFreeEx(handle, addr, 0, MEM_RELEASE);
        return;
    }

    DWORD wait_status = WaitForSingleObject(thread, INFINITE); // INFINITE?
    CloseHandle(thread);
    VirtualFreeEx(handle, addr, 0, MEM_RELEASE);

}

HWND hwnd;
DWORD pid;
HANDLE handle;

template <typename T, size_t size>
void WriteMemory(std::array<T, size> value, std::initializer_list<uintptr_t> addr)
{
    T buff[size] = {0};
    for (size_t i = 0; i < size; i++)
        buff[i] = value[i];
    uintptr_t offset = 0;
    for (auto it = addr.begin(); it != addr.end(); it++)
    {
        if (it != addr.end() - 1)
        {
            SIZE_T read_size = 0;
            int ret = ReadProcessMemory(handle, (const void *)(offset + *it), &offset, sizeof(offset), &read_size);
            if (ret == 0 || sizeof(offset) != read_size)
                return;
        }
        else
        {
            SIZE_T write_size = 0;
            int ret = WriteProcessMemory(handle, (void *)(offset + *it), &buff, sizeof(buff), &write_size);
            if (ret == 0 || sizeof(buff) != write_size)
                return;
        }
    }
}

template <typename T, size_t size>
void enable_hack(HACK<T, size> hack, bool on)
{
    if (hack.mem_addr == 0x00000000 || hack.mem_addr == 0xffffffff)
        return;

    if (on)
        WriteMemory(std::array<T, size>(hack.hack_value), {hack.mem_addr});
    else
        WriteMemory(std::array<T, size>(hack.reset_value), {hack.mem_addr});
}

template <typename T, size_t size>
void enable_hack(std::vector<HACK<T, size>> hacks, bool on)
{
    for (size_t i = 0; i < hacks.size(); i++)
    {
        if (hacks[i].mem_addr == 0x00000000 || hacks[i].mem_addr == 0xffffffff)
            continue;

        if (on)
            WriteMemory(std::array<T, size>(hacks[i].hack_value), {hacks[i].mem_addr});
        else
            WriteMemory(std::array<T, size>(hacks[i].reset_value), {hacks[i].mem_addr});
    }
}


void generate_spawn_list()
{
    asm_init();
    asm_mov_exx_dword_ptr(Reg::EDI, 0x6A9EC0);
    asm_mov_exx_dword_ptr_exx_add(Reg::EDI, 0x768);
    asm_call(0x4092E0);
    asm_ret();
    asm_code_inject(handle);
}

void update_spawn_preview()
{
	HACK<uint8_t,1> hack_street_zombies={0x0043a153, {0x80}, {0x85}}; 
	enable_hack(hack_street_zombies, true);
    asm_init();
    asm_mov_exx_dword_ptr(Reg::EBX, 0x6A9EC0);
    asm_mov_exx_dword_ptr_exx_add(Reg::EBX, 0x768);
    asm_call(0x40DF70);
    asm_mov_exx_dword_ptr(Reg::EAX, 0x6A9EC0);
    asm_mov_exx_dword_ptr_exx_add(Reg::EAX, 0x768);
    asm_mov_exx_dword_ptr_exx_add(Reg::EAX, 0x15c);
	asm_push_exx(Reg::EAX);
    asm_call(0x43A140);
    asm_ret();
    asm_code_inject(handle);
    enable_hack(hack_street_zombies, false);
}

void InternalSpawn(std::array<bool, 33> zombies)
{  
    WriteMemory(zombies, {0x6A9EC0, 0x768, 0x54D4});

    std::array<int, 2000> zombies_list;
    zombies_list.fill(-1);
    WriteMemory(zombies_list, {0x6A9EC0, 0x768, 0x6B4});

    generate_spawn_list();
    //update_spawn_preview();
}
