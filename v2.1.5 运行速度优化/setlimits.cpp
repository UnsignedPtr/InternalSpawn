#include"writelist.h"

struct LIMITS limits[1000];
int limits_max=0;

std::array<bool,33> zombies;

int test_times=0;
char msg[1000];

extern HANDLE handle;

char buf[4*2000];

void check_limits(){
	/*清空act*/
	for(int i=0;i<limits_max;i++){
		limits[i].ActualCount=0;
	}
	/*读出怪列表，统计总数*/
	unsigned pvz_base;
    unsigned main_object;
    ReadProcessMemory(handle, LPCVOID(0x6a9ec0), &pvz_base, 4, NULL);
    ReadProcessMemory(handle, LPCVOID(pvz_base + 0x768), &main_object, 4, NULL);
	unsigned p=0;
	ReadProcessMemory(handle, LPCVOID(main_object + 0x6b4), buf, 4*2000, NULL); //一次性先把2000个数据读取好，优化速度，在循环中反复CALL这个函数会大幅降低运行速度 
    for(int w=1;w<=40;w++){
    	for(int j=1;j<=50;j++){
    		unsigned data=*((int*)(buf+p));
    		if(data!=-1){
    			for(int k=0;k<limits_max;k++){
    				if(limits[k].ZomType==data && w>=limits[k].WaveFrom && w<=limits[k].WaveTo){
    					limits[k].ActualCount++;
					}
				}
			}
    		else {
    			p=w*200; /*下一波的起始处，50*sizeof(int)=200*/
				break;
			}
    		p+=4;
		}
	}
	/*检验限制*/
	char flag=1;
	for(int k=0;k<limits_max;k++){
		if(limits[k].LimitType==0 && limits[k].LimitCount!=limits[k].ActualCount)flag=0;
		if(limits[k].LimitType==1 && limits[k].LimitCount>limits[k].ActualCount)flag=0;
		if(limits[k].LimitType==-1 && limits[k].LimitCount<limits[k].ActualCount)flag=0;
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


char *skipspace(char *p){
	while(*p==' ')p++;
	return p;
}
int parse(std::string ss){
	char s[1000];
	strcpy(s,ss.c_str());
	char *p=s;
	struct LIMITS temp;
	
	p=skipspace(p);
	if(!isdigit(*p))return -1;
	//ZomType
	for(;isdigit(*p);p++){
		temp.ZomType=temp.ZomType*10+*p-'0';
	}
	//LimitType
	p=skipspace(p);
	if(p[0]=='>' && p[1]=='=')p+=2,temp.LimitType=1;
	else if(p[0]=='<' && p[1]=='=')p+=2,temp.LimitType=-1;
	else if(p[0]=='=' && p[1]=='=')p+=2,temp.LimitType=0;
	else if(p[0]=='>')p++,temp.LimitType=2;
	else if(p[0]=='<')p++,temp.LimitType=-2;
	else return -1;
	//>和<转化为>=和<=
	//因为在整数范围内，x>n和x>=n+1、x<n和x<=n-1等价 
	//LimitCount
	p=skipspace(p);
	if(!isdigit(*p))return -1;
	for(;isdigit(*p);p++){
		temp.LimitCount=temp.LimitCount*10+*p-'0';
	}
	if(temp.LimitType==2){
		temp.LimitType=1;temp.LimitCount++;
	}
	if(temp.LimitType==-2){
		temp.LimitType=-1;temp.LimitCount--;
	}
	p=skipspace(p);
	if(*p==0x00){
		temp.WaveFrom=1;temp.WaveTo=40;
	}
	else if(*p==','){ //WaveFrom, WaveTo
		p++;p=skipspace(p);
		int ret=sscanf(p,"%d-%d",&temp.WaveFrom,&temp.WaveTo);
		if(ret==1)temp.WaveTo=temp.WaveFrom;
	}
	else return -1;
	
	
	limits[limits_max]=temp;
	limits_max++;
}
