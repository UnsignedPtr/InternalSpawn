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

extern struct LIMITS limits,act;
extern std::array<bool,33> zombies;
extern int test_times;
extern char msg[1000];

void show_msg();
void InternalSpawn(std::array<bool,33>zombies);
void check_limits();

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
	
	
	for(int i=0;i<33;i++){
		limits.status[i]=1437;
	}
	
	printf("\n����������Ҫ�ĳ�������:\n");
	printf("ÿ����������: ���ƵĽ�ʬ���, ��������(>/>=/</<=/==), ����; �ÿո�ָ�\n");
	printf("������22 <= 6��ʾ����Ͷ��������С�ڵ���6\n");
	printf("������е����ƶ�������ϣ�����-1��Ȼ��س�\n");
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
			sprintf(msg,"δ�ҵ�����Ҫ��ĳ����б�.ˢ�´���:%d\n",test_times);exit(0);
		}
	} 
	return 0;
	
}
