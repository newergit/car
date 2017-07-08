#include  "define_header_file.h"
/*
客户与客户通信:
1.客户必须先与服务端通信一次,并发送客户端唯一识别值
2.服务记录成功请求客户的地址结构,用客户唯一识别值 查找 客户对应的地址结构
3.客户把消息发送给服务端并附上目标客户唯一识别值
4.服务端建立一个UDP套接字,负责消息转发
*/
//定义客户端地址信息
typedef struct  client_i nfo
{
    char client_id[10];  
    struct sockaddr_in client_addr;
    int   client_addr_len;  
}client_info;
//定于通信结构信息
typedef  struct   communication_info
{
    char  dest_client_id[10];  
    char  msg[100];
} communication_info;


//定义链表节点结构
typedef struct  node
{
    client_info  info;
    struct  node  *next;
}node;
int main(int argc,const char* argv[])
{
    int listen_fd=socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd==-1)
    {
        perror("socket  init fail\n");
        exit(-1);
    }
    //改变套接字选项 可以立即重新绑定 最近关闭的 地址+端口值 
    int value=1;
    setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&value,sizeof(listen_fd));
    //初始化地址结构=地址族+地址值+端口值
    struct  sockaddr_in  listen_addr;
    bzero(&listen_addr,sizeof(listen_addr));
    listen_addr.sin_family=AF_INET;
    listen_addr.sin_port=htons(1234);
    //listen_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    listen_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    //listen_addr.sin_addr.s_addr=inet_addr("192.168.117.5");
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


    //初始化UDP 改变套接字
    int udp_fd=socket(AF_INET,SOCK_DGRAM,0);
    if(udp_fd==-1)
    {
        perror("socket  init fail\n");
        exit(-1);
    }
    //绑定udp地址结构
    if(bind(udp_fd,(struct sockaddr*)&listen_addr,sizeof(listen_addr))==-1)
    {
        perror("udp bing error:");
        exit(-1);
    }

    int max_fd=listen_fd>udp_fd?listen_fd:udp_fd;
    fd_set read_fds;
    struct timeval  tv;
    int ret;
    node *head=NULL;
    node* curr=NULL;
    node* last=NULL;
    
    while(1)
    {
        FD_ZERO(&read_fds);
        FD_SET(listen_fd,&read_fds);
        FD_SET(udp_fd,&read_fds);
        tv.tv_sec=1;
        tv.tv_usec=0;
        ret=select(max_fd+1,&read_fds,NULL,NULL,&tv);
        if(ret<=0)
        {
            continue;
        }
        if(ret>0)
        {
            if(FD_ISSET(listen_fd,&read_fds))
            {
                client_info *  c_info=(client_info*)malloc(1*sizeof(client_info));
                c_info->client_addr_len=sizeof(c_info->client_addr);
                int fd=accept(listen_fd,(struct sockaddr*)&c_info->client_addr,(socklen_t*)&(c_info->client_addr_len));
                printf("fd=%d\n",fd);
                if(fd>0)
                {
                    printf("%s\t%d\n",inet_ntoa(c_info->client_addr.sin_addr),ntohs(c_info->client_addr.sin_port));
                    bzero(c_info->client_id,sizeof(c_info->client_id));
                    int r_len=recv(fd,c_info->client_id,sizeof(c_info->client_id)-1,0);
                    if(r_len>0)
                    {
                        send(fd,(void*)&c_info->client_addr,sizeof(c_info->client_addr),0);
                        node* new_node=(node*)malloc(1*sizeof(node));
                        new_node->info=*c_info;
                        printf("id=%s\n",new_node->info.client_id);
                        new_node->next=NULL;
                        if(head==NULL)
                        {
                            head=new_node;
                            last=new_node;
                            new_node=NULL;
                        }
                        else 
                        {
                            last->next=new_node;
                            last=new_node;
                            new_node=NULL;                            
                        }
                    }                    
                }
            }
            if(FD_ISSET(udp_fd,&read_fds))
            {
                communication_info  info;
                struct sockaddr_in   addr_self;
                socklen_t  addr_self_len=sizeof(addr_self);  
                bzero(&info,sizeof(info));
                recvfrom(udp_fd,&info,sizeof(info),0,(struct sockaddr*)&addr_self,&addr_self_len);
                if(head!=NULL)
                {
                    printf("info.id=%s\n",info.dest_client_id);
                    curr=head;
                    while(curr!=NULL)
                    {
                        if(strcmp(curr->info.client_id,info.dest_client_id)==0)
                        {
                            sendto(udp_fd,info.msg,strlen(info.msg),0,(struct sockaddr*)&curr->info.client_addr,curr->info.client_addr_len);
                            break;
                        }
                        curr=curr->next;
                    }
                    if(curr==NULL)
                    {
                         sendto(udp_fd,"not found client",strlen("not found client"),0,(struct sockaddr*)&addr_self,addr_self_len);
                    }
                }
            }
        }
    }
    return 0;
}