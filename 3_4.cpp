/*
	ftp服务器
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>



typedef struct con_fd_addr
{
    //accpet返回的通信套接字
    int nod_fd;
    //对应返回的ip和端口
    struct sockaddr_in nod_addr;
}con_fd_addr;


#define CLIENT_MAX 1024
#define DIRP_MAX_NUM  	100
#define DIRP_MAX_LEN  	256
#define DIR_NAME "../"



//用于存储每一次建立的新连接的fd和addr
con_fd_addr con[CLIENT_MAX] = {0};
int con_index = 0;
int confd = 0;

char * ip0 = NULL;
char * port0 = NULL;


char dirp_arr[DIRP_MAX_NUM][DIRP_MAX_LEN];
int dirp_index = 0;




//用来存放epoll阻塞等待监听后返回的就绪文件描述符的数组
struct epoll_event ep_recv_event[CLIENT_MAX]={0};
struct sockaddr_in accept_recv_addr;



void message_excg(int i);
void show_view();




int tcp_epoll_recv(char * ip,char * port)
{
	ip0 = ip;port0 = port;
    int tcp_sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(tcp_sockfd == -1)
	{
		perror("socket failed!!");
		return -1;
	}

    struct sockaddr_in tcp_addr;
	tcp_addr.sin_family = AF_INET;
	tcp_addr.sin_port=htons(atoi(port));
	tcp_addr.sin_addr.s_addr = inet_addr(ip);
	
    //绑定服务器ip地址
	int ret = bind(tcp_sockfd,(struct sockaddr *)&tcp_addr,sizeof(tcp_addr));
	if(ret == -1)
	{
		perror("bind failed!!");
		close(tcp_sockfd);
		return -1;
	}

    //将该套接字设置为可复用，这样异常中断后就可以迅速连接
    int on = 1;
	int ret1 = setsockopt(tcp_sockfd,SOL_SOCKET,SO_REUSEADDR,(void *)&on,sizeof(on));
	int ret2 = setsockopt(tcp_sockfd,SOL_SOCKET,SO_REUSEPORT,(void *)&on,sizeof(on));
	if(ret1 == -1 || ret2 == -1)
	{
		perror("setsockopt ADDR failed");
		return -1;
	}

    //创建epoll结构体数组，将描述符返回到ep_fd
    int ep_fd = epoll_create(1); //最大epoll最大可以同时监听多少个套接字
	if(ep_fd == -1)
	{
		perror("epoll_create failed");
		close(tcp_sockfd);
		return -1;
	}

    //为加入到epoll数组中的套接字编制好触发事件，同时把套接字也放到epoll结构体数组中去
    struct epoll_event ep_event;
	ep_event.events = EPOLLIN;
	ep_event.data.fd = tcp_sockfd;

    //把服务器套接字加入到epoll结构体数组中
    ret = epoll_ctl(ep_fd,EPOLL_CTL_ADD,tcp_sockfd,&ep_event);
    if(ret == -1)
    {
        perror("epoll_ctl failed");
        close(tcp_sockfd);
        return -1;
    }

    

    //服务器进入监听模式
    ret = listen(tcp_sockfd,CLIENT_MAX);
	if(ret == -1)
	{
		perror("listen failed!!");
		close(tcp_sockfd);
		return -1;
	}

    while(1)
    {
        int ep_recv_num = epoll_wait(ep_fd,ep_recv_event,CLIENT_MAX,-1);
        if(ep_recv_num < 0)
        {
            perror("epoll failed");
            return -1;
        }
        
        if(ep_recv_num > 0)   //有事件相应
        {
            for(int i = 0 ; i < ep_recv_num ; i++)
            {
                if(ep_recv_event[i].events & POLLIN )  
                {
                    if(ep_recv_event[i].data.fd == tcp_sockfd)  //表示有新的客户端连接
                    {
                        //用accept接收连接
                        
                        socklen_t len = sizeof(accept_recv_addr);
                        confd = accept(tcp_sockfd,(struct sockaddr*)&accept_recv_addr,&len);
                        if(-1 == confd)
                        {
                            perror("accept error");
                            continue;
                        }
                        //把新连接的客户端套接字也加入到epoll监听数组中去
                        ep_event.events = EPOLLIN;
	                    ep_event.data.fd = confd;
                        ret = epoll_ctl(ep_fd,EPOLL_CTL_ADD,confd,&ep_event);
                        if(ret == -1)
                        {
                            perror("epoll_ctl failed");
                            close(confd);
                        }
                        //将该新连接的fd和addr存储到con结构体数组中
                        con[con_index].nod_fd = confd;
                        con[con_index++].nod_addr = accept_recv_addr;
						//将客户端初次连接ftp服务器的界面展示出来
						show_view();
                    }   
                    else        //表示epoll返回来的事件对应的不是服务器的套接字，那么就是accept返回的通信套接字了   //此处是真的要通信了，可以添加进程或者线程来处理不同的任务
                    {
                        message_excg(i);
                    }

                }
            }
            
        }
    }    
}


/*
	展示客户端首次连接时的界面

**************************************************************
		[ip][poet]		欢迎您的到来！
				
		FTP服务器[ip][port]诚挚的为您服务！
--------------------------------------------------------------
		请 选 择 您 需 要 的 功 能 ：

		1.下 载 文 件   		2.上 传 文 件
***************************************************************


*/
void show_view()
{
	// printf("**************************************************************\n");
	// printf("[%s][%d],欢迎您的到来！\n\n",inet_ntoa(accept_recv_addr.sin_addr),ntohs(accept_recv_addr.sin_port));
	// printf("	FTP服务器[%s][%s]诚挚的为您服务！\n",ip0,port0);
	// printf("--------------------------------------------------------------\n");
	// printf("		请 选 择 您 需 要 的 功 能 ：\n\n");
	// printf("	1.下 载 文 件   		2.上 传 文 件\n");
	// printf("***************************************************************\n");
//将这些界面给用户发过去
	char str[1024];
	strcpy(str,"**************************************************************\n");
    send(confd,str,strlen(str),0);
	sprintf(str,"[%s][%d],欢迎您的到来！\n\n",inet_ntoa(accept_recv_addr.sin_addr),ntohs(accept_recv_addr.sin_port));
	send(confd,str,strlen(str),0);
	sprintf(str,"	FTP服务器[%s][%s]诚挚的为您服务！\n",ip0,port0);
	send(confd,str,strlen(str),0);
	sprintf(str,"--------------------------------------------------------------\n");
	send(confd,str,strlen(str),0);
	sprintf(str,"		请 选 择 您 需 要 的 功 能 ：\n\n");
	send(confd,str,strlen(str),0);
	sprintf(str,"	1.下 载 文 件   		2.上 传 文 件\n");
	send(confd,str,strlen(str),0);
	sprintf(str,"***************************************************************\n");
	send(confd,str,strlen(str),0);




}


/*
	获取文件，将获取到的文件名保存到dirp_arr[][]中。
*/
void dirp_name_get()
{
	const char * dir_name = DIR_NAME;
	DIR *dir = opendir(dir_name);  // /home/china/xx 
	if(dir == NULL)
	{
		perror("opedndir failed");
	}

	//循环读取目录
	struct dirent *dirp = NULL;
	while(dirp = readdir(dir))
	{
		//跳过“.”和“..”目录
		if(strcmp(dirp->d_name,".") == 0 || strcmp(dirp->d_name,"..") == 0)
		{
			continue;
		}
		else//不是“.”和“..”目录
		{
		//那么就把名字存到dirp_name
			strcpy(dirp_arr[dirp_index++],dirp->d_name);
		}
	}
}



/*
	处理客户端下载文件
*/
void client_download()
{
	int max = dirp_index - 1;
	for(int i = 0 ; i < max ; i++)
	{
		char name[1024];
		sprintf(name,"../%s",dirp_arr[i]);
		int fd = open(name,O_RDONLY);
		if( -1 == fd )
		{
			perror("open error");
			return;
		}
		off_t file_size = lseek(fd,0,SEEK_END);
    	lseek(fd,0,SEEK_SET);
		char data_buf[file_size] = {0};
    	read(fd,data_buf,file_size);



		
	}
}


/*
	处理客户端上传文件
*/



/*
    成功连接之后，触发了事件，表示要进行信息通信了
    传入要处理的epoll返回数组中的下标，表示处理哪一个就好
*/
void message_excg(int i)
{
    char recv_buf[1024]={0};	
    int ret = recv(ep_recv_event[i].data.fd,recv_buf,1023,0);
    //找出这个老连接的地址
    struct sockaddr_in temp_addr;
    for(int j = 0 ; j < con_index ; j++)
    {
        if(ep_recv_event[i].data.fd == con[j].nod_fd)
        {
            temp_addr = con[j].nod_addr;
            break;
        }
    }
    if(ret == 0)    //客户端已退出 已关闭连接
    {
        printf("the old client:[%s][%d] is closed",inet_ntoa(temp_addr.sin_addr),ntohs(temp_addr.sin_port));
        close(ep_recv_event[i].data.fd);   
    }
    if(ret > 0)
    {
		//处理操作函数  收到1 表示客户端想要下载文件 收到2 表示客户端想要上传文件
		//manage_recv(i);
        printf("have a message of [%s][%d]:\n%s",inet_ntoa(temp_addr.sin_addr),ntohs(temp_addr.sin_port),recv_buf);
        const char *buf = "OK!!\n";
        send(ep_recv_event[i].data.fd,buf,strlen(buf),0);
		if(recv_buf[0] == '1')
		{
			printf("客户端下载文件\n");
			dirp_name_get();
			client_download();
		}
		else if(recv_buf[0] == '2')
		{
			printf("客户端上传文件\n");
		}



    }
}




int main(int argc,char * argv[])
{
    char * ip = argv[1];
    char * port = argv[2];
    tcp_epoll_recv(ip,port);
}



















