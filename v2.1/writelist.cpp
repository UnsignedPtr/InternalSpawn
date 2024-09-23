#include<windows.h>
#include<vector>
#include<initializer_list>
#include<array>
#include<time.h>
#include<stdio.h>
#include<string>
#include<iostream>

#include"writelist.h"
extern unsigned char *code;
extern unsigned int length;
extern std::vector<unsigned int> calls_pos;

extern HWND hwnd;
extern DWORD pid;
extern HANDLE handle;

extern struct LIMITS limits[1000];
extern int limits_max;
extern std::array<bool,33> zombies;
extern int test_times;
extern char msg[1000];

void show_msg();
void InternalSpawn(std::array<bool,33>zombies);
void check_limits();
int parse(std::string ss);

int time_limit=20;

int main(int argc,char** argv){
	printf("刷列表时长限制(单位: 秒):\n");
	printf("(可按回车跳过，不设置则默认20秒未刷出即停止本程序)\n");
	char buf[200];
	fgets(buf,sizeof(buf),stdin);
	sscanf(buf,"%d",&time_limit);
	
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
	
	int num=-1,c;
	for(;;){
		c=getchar();
		if(c=='\n'){
			if(num!=-1)zombies[num]=1;
			break;
		}
		else if(isdigit(c)){
			if(num==-1)num=c-'0';
			else num=num*10+c-'0';
		}
		else if(c==' '){
			if(num!=-1)zombies[num]=1;
			num=-1;
		}
	}
	
	printf("\n输入你所想要的出怪限制:\n");
	printf("每行依次输入: 限制的僵尸编号, 限制类型(>/>=/</<=/==), 数量; 用空格分隔\n");
	printf("如输入22 <= 6表示限制投篮的数量小于等于6\n");
	printf("如果所有的限制都输入完毕，输入-1，然后回车\n");
	printf(">");
	for(;;){
		std::string s;
		std::getline(std::cin,s);
		int ret=parse(s);
		if(ret==-1)break;
	}
	//printf("\n%d\n",limits_max);
	//for(;;);
	
	clock_t tm=clock();
	for(;;){
		InternalSpawn(zombies);test_times++;
		check_limits();
		if(clock()-tm>time_limit*CLOCKS_PER_SEC){
			sprintf(msg,"未找到符合要求的出怪列表.刷新次数:%d\n",test_times);exit(0);
		}
	} 
	return 0;
	
}
