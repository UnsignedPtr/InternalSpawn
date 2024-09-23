#include"writelist.h"

struct LIMITS limits,act;

std::array<bool,33> zombies;

int test_times=0;
char msg[1000];

extern HANDLE handle;

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
