/*
 *基于线程池的实现,控制线程接受外来的连接请求，将连接符放入到缓冲区中,工作线程从缓冲区中取出
 */
#include "sbuf.h"
#include "rio.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>

#define NTHREADS 4
#define SBUFSIZE 16
#define LISTENQ 1024
#define MAXLINE 2048

static int cnt;
static sem_t mutex_cnt;

sbuf_t sbuf;

void *thread(void *vargp);

static void init_echo_cnt(void);
static void echo_cnt(int connfd);


int main(int argc, char *argv[])
{
	int port, listenfd, connfd, optval = 1;
	int i, err;
	socklen_t clientlen = sizeof (struct sockaddr_in);
	struct sockaddr_in serveraddr, clientaddr;
	pthread_t tid;

	if (argc != 2){
		fprintf(stderr, "usage:%s <port>\n", argv[0]);
		exit(0);
	}

	port = atoi(argv[1]);
	sbuf_init(&sbuf, SBUFSIZE);

	/*server open listenfd*/
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		fprintf(stderr, "socket error\n");
		exit(EXIT_FAILURE);
	}

	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,(const void*)&optval, sizeof(int)) < 0){
		fprintf(stderr, "setsockopt error\n");
		exit(EXIT_FAILURE);
	}

	bzero((char *)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(port);

	if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0){
		fprintf(stderr, "bind error\n");
		exit(EXIT_FAILURE);
	}

	if (listen(listenfd, LISTENQ) < 0){
		fprintf(stderr, "listen error\n");
		exit(EXIT_FAILURE);
	}

	/*消费线程，创建了一个线程池*/
	for( i = 0; i < LISTENQ; ++i){
		err = pthread_create(&tid, NULL, thread, NULL);
		if (err != 0){
			fprintf(stderr, "pthread_create error\n");
			exit(EXIT_FAILURE);
		}
	}

	while(1){
		/*producer produce connfd, insert it to the buffer*/
		connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
		if (connfd < 0){
			fprintf(stderr, "accept error\n");
			exit(EXIT_FAILURE);
		}
		sbuf_insert(&sbuf, connfd);
	}
}

void *thread(void *vargp)
{
	int ret;
	if ((ret = pthread_detach(pthread_self())) < 0){
		fprintf(stderr, "pthread_detach errorn\n");
		exit(0);
	}
	while(1){
		int connfd = sbuf_remove(&sbuf);
		echo_cnt(connfd);
		if((ret = close(connfd))< 0){
			fprintf(stderr, "close error\n");
			exit(0);
		}
	}
}

static void init_echo_cnt(void)
{
	int err;
	err = sem_init(&mutex_cnt, 0, 1);
	if (err != 0){
		fprintf(stderr, "sem_init mutex_cnt error\n");
		exit(EXIT_FAILURE);
	}

	cnt = 0;
}

static void echo_cnt(int fd)
{
	int n;
	char buf[MAXLINE];
	rio_t rio;              //线程私有的
	static pthread_once_t once = PTHREAD_ONCE_INIT;

	int err;

	/*初始化全局变量*/
	err = pthread_once(&once, init_echo_cnt);
	if (err != 0){
		fprintf(stderr, "pthread_once error\n");
		exit(EXIT_FAILURE);
	}

	rio_readinitb(&rio, fd);
	while((n = rio_readlineb(&rio, buf, MAXLINE)) != 0){
		err = sem_wait(&mutex_cnt);
		if (err != 0){
			fprintf(stderr, "sem_wait error\n");
			exit(EXIT_FAILURE);
		}
		cnt += n;
		printf("thread %d recevied %d (%d total) bytes on fd %d\n",
			(int)pthread_self(), n, cnt, fd);
		
		err = sem_post(&mutex_cnt);
		if (err != 0){
			fprintf(stderr, "sem_post error\n");
			exit(EXIT_FAILURE);
		}
		rio_writen(fd, buf, n);
	}
}
