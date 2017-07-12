#include "define_head_file.h"

char flag[1024];
int sum=0;

void  pthread_cli(const int* client_fd1)
{
    const int fd=*client_fd1;
    printf("The client is connected\n");
    char buf[1024];
    while(1)
    {
        bzero(buf,sizeof(buf));
        //printf("%d\n",fd);
        recv(fd,buf,sizeof(buf),0);
        //printf("%d\n",*client_fd);
        if (strcmp(buf,"8")==0)
        {
            strcpy(flag,"0");
            sum=sum+1;
            printf("The client is out\n");
            break;
        }
        //printf("%s\n",buf);
        //sleep(1);
        //send(*client_fd,buf,sizeof(buf),0);
        strcpy(flag,buf);
        sum=sum+1;
    }
}

void  pthread_car(const int* client_fd2)
{
    const int fd=*client_fd2;
    printf("The car is connected\n");
    while(1)
    { 
        if(sum>0)
        {
            //printf("%d\n",*client_fd2);
            //printf("transmit\n");
            send(fd,flag,strlen(flag),0);
            sum=sum-1;
        }
    //sleep(1);
    }
}

int main(int argc,char const *argv[])
{
	int listen_fd=socket(AF_INET,SOCK_STREAM,0);
	if(listen_fd==-1)
	{
		perror("socket init fail\n");
		exit(-1);
	}
	//改变套接字选项 可以立即重新绑定 最近关闭的 地址+端口值    
    int value=1;
    setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&value,sizeof(listen_fd));
    //初始化地址结构
	struct  sockaddr_in  listen_addr;
    bzero(&listen_addr,sizeof(listen_addr));
    listen_addr.sin_family=AF_INET;
    listen_addr.sin_port=htons(6666);
    //listen_addr.sin_addr.s_addr=INADDR_ANY;
    listen_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    //绑定
    if(bind(listen_fd,(struct sockaddr*)&listen_addr,sizeof(listen_addr))==-1)
    {
        perror("bind fail\n");
        exit(-1);
    }
    //监听
    if(listen(listen_fd,10)==-1)
    {
        perror("listen fail\n");
        exit(-1);
    }
    printf("Server initialization of success!\n");
    //接收客户请求
    while(1)
    {
        int i=0;
    	struct  sockaddr_in  client_addr;
        memset(&client_addr,0,sizeof(client_addr));
        socklen_t  client_len=sizeof(client_addr);
        int client_fd[10];
        client_fd[i]=accept(listen_fd,(struct sockaddr*)&client_addr,&client_len);
        if(client_fd>0)
        {
            printf("client_fd %d\n", client_fd[i]);
        	//这里段错误
			//printf("ip=%s \t port=%d \n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
			//char buf[1024];
		    char ID[4];
		    //bzero(buf,sizeof(buf));
		    bzero(ID,sizeof(ID));
		    recv(client_fd[i],ID,sizeof(ID),0);
            pthread_t  pthreadcli;
            pthread_t  pthreadcar;
		    if (strcmp(ID,"1")==0)
		    {
                //printf("100 %d\n",client_fd[i]);
                int ret=pthread_create(&pthreadcli,NULL,(void*)pthread_cli,&client_fd[i++]);
                if(ret!=0)
                {
                    perror("pthread_creat error\n");
                    exit(-1);
                }
		    }
            if (strcmp(ID,"2")==0)
            {
                //printf("110 %d\n",client_fd[i]);
                int ret=pthread_create(&pthreadcar,NULL,(void*)pthread_car,&client_fd[i++]);
                //pthread_join(pthreadcar,NULL);
                if(ret!=0)
                {
                    perror("pthread_creat error\n");
                    exit(-1);
                }
            }
            //pthread_join(pthreadcli,NULL);
            //pthread_join(pthreadcar,NULL);
        }
    }
}
        
