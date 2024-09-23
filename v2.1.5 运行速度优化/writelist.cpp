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
	printf("ˢ�б�ʱ������(��λ: ��):\n");
	printf("(�ɰ��س���������������Ĭ��20��δˢ����ֹͣ������)\n");
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
	
	printf("[HINT]��ʬ��Ŷ�Ӧ��:\n");
	std::string s[]={
		"�ս�","����","·��","�Ÿ�","��Ͱ",
		"����","����","���","����","����",
		"Ѽ��","Ǳˮ","����","ѩ��","����",
		"С��","����","��","����","ѩ��",
		"�ļ�","����","Ͷ��","����","С��",
		"����","�㶹","���","����","��ǹ",
		"�ѹ�","�߼�","����",
	};
	for(int i=0;i<33;i++){
		printf("[%02d]%s\t",i,s[i].c_str());
		if(i%4==3)printf("\n");
	}
	printf("\n����һ�г�����������Ӧ�Ľ�ʬ���, �ÿո�ָ�.\n");
	printf("����: ���ֹؿ��г������Ͳ������ս��ᵼ����Ϸ����, ������ע��.\n");
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
	
	printf("\n����������Ҫ�ĳ�������:\n");
	printf("ÿ����������: ���ƵĽ�ʬ���, ��������(>/>=/</<=/==), ����; �ÿո�ָ�\n");
	printf("������22 <= 6��ʾ����Ͷ��������С�ڵ���6\n");
	printf("������е����ƶ�������ϣ�����-1��Ȼ��س�\n");
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
			sprintf(msg,"δ�ҵ�����Ҫ��ĳ����б�.ˢ�´���:%d\n",test_times);exit(0);
		}
	} 
	return 0;
	
}
