/*
 *基于进程的并发，父子进程共享文件表，但是不共享进程地址空间，进程间的通信
 *将消耗很多资源
 */
#include "csapp.h"


void sigchld_handler(int sig){
	while (waitpid(-1, 0, WHOHANG) > 0)
		;
}

void echo(int con);

int main(int argc, char *argv[]){
	int listenfd, connfd, port;
	socklen_t clientlen = sizeof(struct sockaddr_in);
	struct sockaddr_in clientaddr;

	if (argc != 2){
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(0);
	}
	port = atoi(argv[1]);

	listenfd = Open_listenfd(port);
	
	while(1){
		connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
		if (Fork == 0){
			Close(listenfd);
			echo(connfd);
			Close(connfd);
			exit(0);
		}
		Close(connfd);
	}
}
