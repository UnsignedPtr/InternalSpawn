#include<windows.h>
#include<vector>
#include<initializer_list>
#include<array>
#include<time.h>
#include<stdio.h>
#include<string>
#include<iostream>

/*asmlib.cpp*/
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

/*setlimits.cpp*/
struct LIMITS{
	int ZomType;
	int LimitType;//1大于，-1小于，0等于 
	int LimitCount;
	int ActualCount;
	int WaveFrom, WaveTo;
	LIMITS(){
		ZomType=0;LimitType=0;LimitCount=0;ActualCount=0;WaveFrom=0;WaveTo=0;
	}
};
