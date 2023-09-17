/* 
    开发板(客户端)
    tcp客户端
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int client_sockfd;
unsigned char cmd[1024] = {0};	//发送指令

/*
	用于服务端成功连接之后进行的消息交换
*/
void client_message_excg(char * ip,char * port)
{
    char send_buf[1024]={0};
    fgets(send_buf,1023,stdin);
    send(client_sockfd,send_buf,strlen(send_buf),0);		//服务器那个accept后面也有个recv，所以这个send就算发了请求，服务器那边也不会处理。。 所以服务器检测到事件发送时tcp_socket的时候，就用accept就好了，不用调用recv
    char recv_buf[1024]={0};
    recv(client_sockfd,recv_buf,1023,0);
    printf("recv data[%s][%s]:\n%s\n",ip,port,recv_buf);
}


int tcp_client(char * ip,char * port)
{
	//1.创建套接字 
	client_sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(client_sockfd == -1)
	{
		perror("socket failed!!");
		return -1;
	}

	//2.指定服务器的地址
	struct sockaddr_in ser_addr;
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port=htons(atoi(port));
	ser_addr.sin_addr.s_addr = inet_addr(ip);
	//3.连接服务器
	int ret = connect(client_sockfd,(struct sockaddr *)&ser_addr,sizeof(ser_addr));
	if(ret == -1)
	{
		perror("connect failed!!");
		return -1;
	}
	while(1)	//连接成功，进行信息交换
	{
		client_message_excg(ip,port);
	}
	//4.关闭套接字
	shutdown(client_sockfd,SHUT_RDWR);
	return 0;
}


int main(int argc,char * argv[])
{
    tcp_client(argv[1],argv[2]);

    return 0;
}








