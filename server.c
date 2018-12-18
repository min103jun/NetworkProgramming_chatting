//2013105016 김민준
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#define BUFSIZE 1024

typedef struct user {
	char nickname[30];
	char room[10];
} user;

void error_handling(char*);

int main(int argc, char** argv)
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr; 
    struct sockaddr_in clnt_addr;
    struct timeval timeout;
    fd_set reads, cpyReads;
    socklen_t addrSz;
    int fdMax, str_len, fdNum, i, j;
    char buf[BUFSIZE];
	char token[30], temp[BUFSIZE], whisper[BUFSIZE], oldname[30]; 
	user userinfo[64];
	
	memset(userinfo, 0, sizeof(user)*64);
	for(i = 0; i < 64; i++) {
		strcpy(userinfo[i].nickname, "noname");
	}

    if(argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    if((serv_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
        error_handling("socket() error");
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == - 1)
        error_handling("bind() error");

    if(listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    FD_ZERO(&reads);
    FD_SET(serv_sock, &reads);
    fdMax = serv_sock;

    while(1) {
		cpyReads = reads;
		timeout.tv_sec = 5;
		timeout.tv_usec = 5000; 

		if((fdNum = select(fdMax + 1, &cpyReads, 0, 0, &timeout)) == -1)
			break;
		if(fdNum == 0)
			continue;

		for(i = 0; i < fdMax +1; i++) {
			if(FD_ISSET(i, &cpyReads)) {
				if(i == serv_sock) {
					addrSz = sizeof(clnt_addr);
					clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &addrSz);
					FD_SET(clnt_sock, &reads);
					if(fdMax < clnt_sock)
						fdMax = clnt_sock;
					printf("connected client : %d \n", clnt_sock - 3); 	
				}
				else {
						str_len = read(i, buf, BUFSIZE);
						if(str_len == 0) {
							for(j = 4; j < fdMax + 1; j++) {
								strcpy(temp, userinfo[i - 4].nickname);
								strcat(temp, " is closed\n");
								if(i != j && strncmp(userinfo[i-4].room, userinfo[j-4].room, 1) == 0) {	
									write(j, temp, BUFSIZE);	
								}
							}
							memset(temp, 0, sizeof(temp));
							memset(buf, 0, sizeof(buf));
							memset(&userinfo[i - 4], 0, sizeof(user));
							strcpy(userinfo[i - 4].nickname, "noname"); 
							FD_CLR(i, &reads);
							close(i);
							printf("closed client : %d \n", i - 3);
						} // 클라이언트 종료시 초기화.
						else {
							if(strncmp(buf, "/room", 5) == 0) {	
								memset(userinfo[i - 4].room, 0, sizeof(userinfo[i - 4].room));
								strtok(buf, " ");
								strcpy(userinfo[i - 4].room, strtok(NULL, " "));
								strcpy(temp, "Notice : Enter room ");
								strcat(temp, userinfo[i - 4].room);
								write(i, temp, BUFSIZE);
								userinfo[i - 4].room[1] = '\0';
								memset(temp, 0, sizeof(temp));
								memset(buf, 0, sizeof(buf));
								for(j = 4; j < fdMax + 1; j++) {
									strcpy(temp, "Notice : ");
									strcat(temp, userinfo[i - 4].nickname);
									strcat(temp, " is enter\n");
									if(i != j && strncmp(userinfo[i-4].room, userinfo[j-4].room, 1) == 0) {	
										write(j, temp, BUFSIZE);	
									}
								}
								memset(temp, 0, sizeof(temp));
								printf("client %d room : %s   nickname : %s\n", i - 3, userinfo[i - 4].room, userinfo[i - 4].nickname);
							}
							else if(strncmp(buf, "/nickname", 9) == 0) {
								strcpy(oldname, userinfo[i - 4].nickname);
								memset(userinfo[i - 4].nickname, 0, sizeof(userinfo[i - 4].nickname));
								strtok(buf, " ");
								strcpy(userinfo[i - 4].nickname, strtok(NULL, " "));
								strcpy(temp, "Notice : Change your nickname ");
								strcat(temp, userinfo[i - 4].nickname);
								write(i, temp, 50);
								userinfo[i - 4].nickname[strlen(userinfo[i - 4].nickname) - 1] = '\0';
								memset(temp, 0, sizeof(temp));
								memset(buf, 0, sizeof(buf));
								for(j = 4; j < fdMax + 1; j++) {
									strcpy(temp, "Notice : ");
									strcat(temp, oldname);
									strcat(temp, " is change nickname -> ");
									strcat(temp, userinfo[i - 4].nickname);
									strcat(temp, "\n");
									if(i != j && strncmp(userinfo[i-4].room, userinfo[j-4].room, 1) == 0) {	
										write(j, temp, BUFSIZE);	
									}
								}
								memset(temp, 0, sizeof(temp));
								printf("client %d room : %s   nickname : %s\n", i - 3, userinfo[i - 4].room, userinfo[i - 4].nickname);
							}
							else if(strncmp(buf, "/w", 2) == 0) {
								strtok(buf, " ");
								strcpy(whisper, strtok(NULL, " "));
								strcpy(buf, strtok(NULL, ""));
								for(j = 4; j < fdMax + 1; j++) {
									if(i != j && strcmp(whisper, userinfo[j - 4].nickname) == 0) {
										strcpy(temp, "(whisper) ");
										strcat(temp, userinfo[i - 4].nickname);
										if(strcmp(userinfo[i - 4].room,userinfo[j - 4].room) != 0) {
											strcat(temp, " (from room ");
											strcat(temp, userinfo[i - 4].room);
											strcat(temp, ")");
										}
										strcat(temp, " : ");
										strcat(temp, buf);
										write(j, temp, BUFSIZE);
										memset(temp, 0, sizeof(temp));
										memset(buf, 0, sizeof(buf));
										memset(whisper, 0, sizeof(whisper));
									} 
								}
							}
							else {
								for(j = 4; j < fdMax + 1; j++) {
									strcpy(temp, userinfo[i - 4].nickname);
									strcat(temp, " : ");
									strcat(temp, buf);
									if(i != j && strncmp(userinfo[i-4].room, userinfo[j-4].room, 1) == 0) {	
										write(j, temp, BUFSIZE);
									} //자신을 제외한 모두에게 받은 메세지 출력함*/
								}
								memset(temp, 0, sizeof(temp));
								memset(buf, 0, sizeof(buf));
							}
						}
				}
			}
		}
	}

	close(serv_sock);
	return 0;
}

void error_handling(char*buf) {
    fputs(buf, stderr);
    fputc('\n', stderr);
    exit(1);
}
