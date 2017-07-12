#include "define_head_file.h"

int main(int argc,char const *argv[])
{
	printf("正在寻找服务器\n");
	int client_fd=socket(AF_INET,SOCK_STREAM,0);
	if(client_fd==-1)
	{
		perror("soket fail\n");
		exit(-1);
	}
	//定义服务端地址结构
	struct  sockaddr_in  ser_addr;
	ser_addr.sin_family=AF_INET;
	ser_addr.sin_port=htons(6666);
	//ser_addr.sin_addr.s_addr=INADDR_ANY;
	ser_addr.sin_addr.s_addr=inet_addr("47.95.9.185");
	//客户端发送链接请求
	if(connect(client_fd,(struct sockaddr*)&ser_addr,sizeof(ser_addr))==-1)
	{
		printf("连接服务器失败\n");
		exit(-1);
	}
	printf("已成功连接服务器\n");
	char ID[4]="1";
	send(client_fd,ID,strlen(ID),0);
	char buf[1024];
	printf("************\n0.暂停\n1.前进\n2.后退\n3.向前左转\n4.向前右转\n5.向后左转\n6.向后右转\n7.自动驾驶\n8.退出\n************\n");
	while(1)
	{
		bzero(buf,sizeof(buf));
    	gets(buf);
    	send(client_fd,buf,strlen(buf),0);
    	if (strcmp(buf,"8")==0)
    		exit(-1);
    	//bzero(buf,sizeof(buf));
    	//recv(client_fd,buf,sizeof(buf),0);
    	//printf("服务器已收到指令%s\n",buf);
    }
}
