/*
 *基于io多路复用的并发编程
 */

typedef struct{
	int maxfd;
	fd_set read_set;
	fd_set ready_set;
	int ready;
	int maxi;
	int clientfd[FD_SETSIZE];
	rio_t clientrio[FD_SETSIZE];
}pool;

int byte_cnt = 0;

void init_pool(int fd, pool *p){
	int i;
	p->maxfd = -1;
	for( i = 0; i < FD_SETSIZE; i++)
		p->clientfd[i] = -1;
	
	p->maxfd = fd;
	FD_ZERO(&p->read_set);
	FD_SET(fd, &p->read_set);
}

void add_client(int fd, pool *p){
	int i;
	p->nready--;
	for(i = 0; i < FD_SETSIZE; ++i)
		if (clientfd[i] < 0){
			clientfd[i] = fd;
			Rio_readinitb(&p->clientrio[i], fd);

			FD_SET(fd, &p->read_set);

			if (fd > p->maxfd)
				p->maxfd = fd;
			if (i > p->maxi)
				p->maxi = i;
			break;
		}
	if(i  == FD_SETSIZE)
		fprintf(stderr, "add_client error: Too many clinets");
}

void check_client(pool *p)
{
	int i, connfd, n;
	rio_t rio;
	char buf[MAXLINE];

	for(i = 0; (i <= p->maxi) && (p->nready > 0); ++i){
		
		connfd = p->clientfd[i];
		rio = p->clientrio[i];

		if((connfd > 0) && (FD_ISSET(connfd, &p->ready_set))){
			p->nready--;
			if( (n = Rio_readlineb(&rio, buf, MAXLINE)) != 0){
				byte_cnt += n;
				printf("Server received %d (%d total) bytes on fd %d\n",
					n, byte_cnt, connfd);
				Rio_writen(connfd, buf, n);
			}

			else{
				Close(connfd);
				FD_CLR(connfd, &p>readset);
				p->clientfd[i] = -1;
			}
		}
	}
}

int main(int argc, char *argv[])
{
	int listenfd, connfd, port;
	socklen_t clientlen = sizeof(struct sockaddr_in);
	struct sockaddr_in clientaddr;
	static pool pool;

	if (argc != 2){
		frpintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(0);
	}

	port = atoi(argv[1]);

	listenfd = Open_listenfd(port);

	init_pool(listenfd, &pool);

	while(1){
		pool.ready_set = pool.read_set;
		pool.nready = Select(pool.maxfd + 1, &pool.ready_set, NULL, NULL, NULL);

		if (FD_ISSET(listenfd, &pool.ready_set)){
			connfd = Accept(listenfd, (SA *)clientaddr, &clientlen);
			add_client(connfd, &pool);
		}

		check_client(&pool);
	}
}	
