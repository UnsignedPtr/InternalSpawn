#include<windows.h>
#include<vector>
#include<initializer_list>
#include<array>
#include<time.h>
#include<stdio.h>
#include<string>
#include<iostream>


typedef void *HANDLE;

template <typename T, size_t size>
struct HACK
{
    uintptr_t mem_addr;
    std::array<T, size> hack_value;
    std::array<T, size> reset_value;
};

enum class Reg : unsigned int
{
    EAX = 0,
    EBX = 3,
    ECX = 1,
    EDX = 2,
    ESI = 6,
    EDI = 7,
    EBP = 5,
    ESP = 4,
};

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

struct LIMITS{
	int status[33];  
	int count[33];
}limits,act;

std::array<bool,33> zombies;

int test_times=0;
char msg[1000];


void check_limits(){
	/*清空act*/
	for(int i=0;i<33;i++){
		act.count[i]=0;
	} 
	/*读出怪列表，统计总数*/
	unsigned pvz_base;
    unsigned main_object;
    ReadProcessMemory(handle, LPCVOID(0x6a9ec0), &pvz_base, 4, NULL);
    ReadProcessMemory(handle, LPCVOID(pvz_base + 0x768), &main_object, 4, NULL);
	unsigned p=0;
    for(int i=1;i<=40;i++){
    	int flag=1;
    	for(int j=1;j<=50;j++){
    		unsigned data;
    		ReadProcessMemory(handle, LPCVOID(main_object + 0x6b4 + p), &data, 4, NULL);
    		if(data!=-1 && flag!=0){
    			act.count[data]++;
    			//if(limits.status[data]==0) if(limits.count[data]!=act.count[data])return;
    			//if(limits.status[data]==1) if(limits.count[data]>act.count[data])return;
    			//if(limits.status[data]==-1) if(limits.count[data]<act.count[data])return;
			}
    		else {
    			flag=0;p+=4;continue;
			}
    		p+=4;
		}
	}
	/*检验限制*/
	char flag=1;
	for(int i=0;i<33;i++){
		if(limits.status[i]==1437)continue;
		if(limits.status[i]==0){
			if(limits.count[i]!=act.count[i])flag=0;
		}
		if(limits.status[i]==1){
			if(limits.count[i]>act.count[i])flag=0;
		}
		if(limits.status[i]==-1){
			if(limits.count[i]<act.count[i])flag=0;
		}
		if(flag==0)return;
	}
	if(flag==1){
		//update_spawn_preview();
		sprintf(msg,"已找到符合要求的出怪列表.刷新次数:%d\n",test_times);
		exit(0);
	}
	
}

void show_msg(){
	MessageBox(NULL,msg,"提示",MB_OK);
}
int main(int argc,char** argv){
	
	atexit(show_msg);
	unsigned int page = 256; // 1MB
    code = new unsigned char[4096 * page];
    length = 0;
    calls_pos.clear();
	
	hwnd = FindWindowA("MainWindow", "Plants vs. Zombies");
    GetWindowThreadProcessId(hwnd, &pid);
    handle = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
    
	
	for(std::array<bool,33>::iterator it=zombies.begin();it!=zombies.end();it++){
		*it=0;
	}
	
	printf("[HINT]僵尸编号对应表:\n");
	std::string s[]={
		"普僵","旗帜","路障","撑杆","铁桶",
		"读报","铁门","橄榄","舞王","伴舞",
		"鸭子","潜水","冰车","雪橇","海豚",
		"小丑","气球","矿工","跳跳","雪人",
		"蹦极","扶梯","投篮","白眼","小鬼",
		"僵王","豌豆","坚果","辣椒","机枪",
		"窝瓜","高坚","红眼",
	};
	for(int i=0;i<33;i++){
		printf("[%02d]%s\t",i,s[i].c_str());
		if(i%4==3)printf("\n");
	}
	printf("\n输入一行出怪类型所对应的僵尸编号, 用空格分隔.\n");
	printf("警告: 部分关卡中出怪类型不设置普僵会导致游戏崩溃, 请自行注意.\n");
	printf(">");
	for(;;){
		int x;scanf("%d",&x);zombies[x]=1;
		if(getchar()=='\n')break;
	}
	
	
	for(int i=0;i<33;i++){
		limits.status[i]=1437;
	}
	
	printf("\n输入你所想要的出怪限制:\n");
	printf("每行依次输入: 限制的僵尸编号, 限制类型(>/>=/</<=/==), 数量; 用空格分隔\n");
	printf("如输入22 <= 6表示限制投篮的数量小于等于6\n");
	printf("如果所有的限制都输入完毕，输入-1，然后回车\n");
	printf(">");
	for(;;){
		int x;scanf("%d",&x);
		if(x==-1)break;
		int t;std::string s;
		std::cin>>s>>t;
		if(s==">"){limits.count[x]=t+1;limits.status[x]=1;}
		if(s=="<"){limits.count[x]=t-1;limits.status[x]=-1;}
		if(s==">="){limits.count[x]=t;limits.status[x]=1;}
		if(s=="<="){limits.count[x]=t;limits.status[x]=-1;}
		if(s=="=="){limits.count[x]=t;limits.status[x]=0;}
	}
	
	
	clock_t tm=clock();
	for(;;){
		InternalSpawn(zombies);test_times++;
		check_limits();
		if(clock()-tm>20*CLOCKS_PER_SEC){
			sprintf(msg,"未找到符合要求的出怪列表.刷新次数:%d\n",test_times);exit(0);
		}
	} 
	return 0;
	
}
