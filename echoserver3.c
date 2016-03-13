/*
 *基于io多路复用的并发编程
 */

#include "csapp.h"

int main(int argc, char *argv[])
{
	int listenfd, port;
	fd_set read_set, ready_set;
	socklen_t clientlen = sizeof(struct sockaddr_in);
	struct sockaddr_in clientaddr;

	if (argc != 2){
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(0);
	}
	port = atoi(port);

	listenfd = Open_listenfd(port);

	FD_ZERO(&readset);
	FD_SET(STDIN_FILENO, &read_set);
	FD_SET(listenfd, &read_set);

	while(1){
		ready_set = read_set;
		Select(listenfd +1, &ready_set, NULL, NULL, NULL);
		if(FD_ISSET(STDIN_FILENO, &ready_set))
			command();
		if(FD_ISSET(listenfd, &ready_set)){
			connfd = Accept(listenfd, (SA *)clientaddr, &clientlen);
			echo(connfd);
			Close(connfd);
		}
	}

}

void command(){
	char buf[MAXLINE];
	if(Fgets(buf, MAXLINE, stdin))
		exit(0);
	printf("%s", buf);
}
