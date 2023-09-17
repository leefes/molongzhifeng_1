/*
    ftp客户端
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int tcp_client(char * ip,char * port)
{
	//1.创建套接字 
	int client_sockfd = socket(AF_INET,SOCK_STREAM,0);
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
	while(1)
	{
    //接收
        sleep(0.1);
        char recv_buf[1024]={0};
		recv(client_sockfd,recv_buf,1023,0);
		printf("%s",recv_buf);
    //发送
        char send_buf[1024]={0};
		fgets(send_buf,1023,stdin);
		send(client_sockfd,send_buf,strlen(send_buf),0);
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



















