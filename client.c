#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
	
#define BUF_SIZE 1024

void * send_msg(void * arg);
void * recv_msg(void * arg);
void error_handling(char * msg);
void first();

int main(int argc, char *argv[])
{
	int sock, str_len;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread;
	void * thread_return;
	char room[10], enter_room[30] = "/room ";

	if(argc!=3) {
		printf("Usage : %s <IP> <port> <name>\n", argv[0]);
		exit(1);
	 }
	
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if(sock == -1)
		error_handling("socket() error");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	  
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");
	
	first();

	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);

	close(sock);
	return 0;
}

void * send_msg(void * arg)   
{
	int sock=*((int*)arg);
	char msg[BUF_SIZE];

	while(1) 
	{
		memset(msg, 0, sizeof(msg));
		fgets(msg, BUF_SIZE-1, stdin);
		
		if(!strcmp(msg,"q\n")||!strcmp(msg,"Q\n")) 
		{
			close(sock);
			exit(0);
		}
		if(write(sock, msg, strlen(msg))==-1)
			error_handling("write() error");
	}
	return NULL;
}

void * recv_msg(void * arg) 
{
	int sock=*((int*)arg);
	char temp_buf[BUF_SIZE];
	int str_len;

	while(1)
	{
		str_len = read(sock, temp_buf, BUF_SIZE-1);
		temp_buf[str_len] = '\0';
		printf("%s", temp_buf);
	}
	return NULL;
}

void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

void first()
{
	printf("****************************************************\n");
	printf("		채팅창 사용법 						 \n");
	printf("1. 채팅방을 바꿀땐 /room #room을 사용하세요			  \n");
	printf("2. 기본 nickname은 다 noname입니다.					  \n");
	printf("3. /nickname 을 입력하시면 nickname 변경이 됩니다.    \n");
	printf("4. /w nickname message 입력하시면 귓속말로 보냅니다.   \n");
	printf("*****************************************************\n");
	printf("													 \n");
	printf("입장하실 /room #room을 입력해 주세요\n");
}