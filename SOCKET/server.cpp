#include "../MyThreadPool/ThreadPool.h"
#include "../MyThreadPool/ThreadPool.cpp"
#include "../MyThreadPool/TaskQueue.h"
#include "../MyThreadPool/TaskQueue.cpp"
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<sys/types.h>


typedef struct SockInfo{
struct sockaddr_in addr;
int cfd;
}SockInfo;

typedef  SockInfo T;

typedef struct PoolInfo{
ThreadPool<T> *pool;
int fd;
PoolInfo():pool(nullptr),fd(-1){}
PoolInfo(ThreadPool<T> *pool,int fd):pool(pool),fd(fd){}
}PoolInfo;


void working(void* arg);
void acceptConn(void* arg);


int main(){

    //1.create monitor socket
    //ipv4
    int fd=socket(AF_INET,SOCK_STREAM,0);
    if(fd==-1){
        perror("socket");
        return -1;
    }
    
// sizeof(struct sockaddr) == sizeof(struct sockaddr_in)
// struct sockaddr_in
// {
//     sa_family_t sin_family;		/* 地址族协议: AF_INET */
//     in_port_t sin_port;         /* 端口, 2字节-> 大端  */
//     struct in_addr sin_addr;    /* IP地址, 4字节 -> 大端  */
//     /* 填充 8字节 */
//     unsigned char sin_zero[sizeof (struct sockaddr) - sizeof(sin_family) -
//                sizeof (in_port_t) - sizeof (struct in_addr)];
// }; 
    //bind locak IP_PORT
    struct sockaddr_in saddr;
    saddr.sin_family=AF_INET;
    //5k up IPPORT host(small)==>net(big)
    saddr.sin_port=htons(10000);
    //bind host ip
    saddr.sin_addr.s_addr=INADDR_ANY;//0=0.0.0.0
    //auto read local ip
    //2.
    int ret=bind(fd,(struct sockaddr*)&saddr,sizeof(saddr));
    if(ret==-1){
    perror("bind");
    return -1;
    }
//3.listen  the max num 128 request
    ret=listen(fd,128);
    if(ret==-1){
        perror("listen");
        return -1;
    }
    //create ThreadPool
    ThreadPool<T> pool(3,10);
    PoolInfo *poolinfo=new PoolInfo(&pool,fd);
    acceptConn(poolinfo);
    return 0;
}
void acceptConn(void* arg){
    //4.block and wait for client

    PoolInfo *poolinfo=(PoolInfo*)arg;
    printf("----------%d",poolinfo->fd);
    socklen_t addrlen=sizeof(struct sockaddr_in);
    while(1){
    struct SockInfo *sinfo=new SockInfo();
    int cfd=accept(poolinfo->fd,(struct sockaddr*)&sinfo->addr,&addrlen);
    sinfo->cfd=cfd;
    if(cfd==-1){
        perror("accept");
        break;
    }
    //threads:
    // pthread_t tid;
    // pthread_create(&tid,NULL,working,sinfo);
    // pthread_detach(tid);

    //ThreadPool
    poolinfo->pool->addTask(Task<T>(working,sinfo));
    //cfd destroy in child thread
    }//while
close(poolinfo->fd);
}

 void working(void* arg){   

     //arg :caddr cfd
    //connect success
    struct SockInfo *pinfo=(struct SockInfo*)arg;
    char ip[32]={0};
    printf("client ip:%s,port:%d\n",
    inet_ntop(AF_INET,&pinfo->addr.sin_addr.s_addr
    ,ip,sizeof(ip)),ntohs(pinfo->addr.sin_port));
    
    //write ans read 
    while(1){
    char buff[1024];
    int len=recv(pinfo->cfd,buff,sizeof(buff),0);
    if(len>0){
    printf("client say:%s\n",buff);
    write(pinfo->cfd,buff,len);
    }else if(len==0){
        printf("client is not connect now...\n");
        break;
    }
    else{
        perror("recv");
        break;
    }
    }//while

//close
//1.listen_fd accpet_fd
close(pinfo->cfd);
}