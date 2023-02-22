#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <wait.h>
 
// TCP客户端连服务端的函数，serverip-服务端ip，port通信端口
//封装socket、gethostbyname、sockaddr_in、connect
int connecttoserver(const char *serverip,const int port);

// n Connections
void NConnection(int n);
 
int main()
{
	NConnection(3);
  return 0;
}

void NConnection(int n){
  // pid_t pid;
  // int i;
  // for(i=0; i<n; i++){
  //   pid=fork();
  //   if(pid<0){
  //     std::cout<<"fork() error"<<std::endl;
  //   }
  // }
  // if(i==n){
  //   int wa=wait(NULL);
  //   if(wa<0){
  //     std::cout<<"wait error"<<std::endl;
  //   }
  //   std::cout<<"father"<<std::endl;
  // }else{
    // 向服务器发起连接请求
    int sockfd=connecttoserver("127.0.0.1",9006);
    if (sockfd<=0) { printf("连接服务器失败，程序退出。\n");  }
    
    char strbuffer[1024];
    
    // 与服务端通信，发送一个报文后等待回复，然后再发下一个报文。
    for (int ii=0;ii<5;ii++)
    {
      memset(strbuffer,0,sizeof(strbuffer));
      scanf("%s", strbuffer);
      // sprintf(strbuffer,"这是第%d个消息，编号%03d。",ii+1,ii+1);
      if (send(sockfd,strbuffer,strlen(strbuffer),0)<=0) break;
      printf("发送：%s\n",strbuffer);
  
      memset(strbuffer,0,sizeof(strbuffer));
      // if (recv(sockfd,strbuffer,sizeof(strbuffer),0)<=0) break;
      printf("接收：%s\n",strbuffer);
      sleep(1);
    }
    
    close(sockfd);
  // }
}
// TCP客户端连服务端的函数，serverip-服务端ip，port通信端口
// 返回值：成功返回已连接socket，失败返回-1。
int connecttoserver(const char *serverip,const int port)
{
  int sockfd = socket(AF_INET,SOCK_STREAM,0); // 创建客户端的socket
 
  struct hostent* h; // ip地址信息的数据结构
  if ( (h = gethostbyname(serverip)) == 0 )
  { perror("gethostbyname"); close(sockfd); return -1; }
 
  // 把服务器的地址和端口转换为数据结构
  struct sockaddr_in servaddr;
  memset(&servaddr,0,sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  memcpy(&servaddr.sin_addr,h->h_addr,h->h_length);
 
  // 向服务器发起连接请求
  if (connect(sockfd, (struct sockaddr *)&servaddr,sizeof(servaddr)) != 0)
  { perror("connect"); close(sockfd); return -1; }
 
  return sockfd;
}
//g++ -o client client.cc -g