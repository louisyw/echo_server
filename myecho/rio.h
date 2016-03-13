#include <unistd.h>
#include <stddef.h>
#include <errno.h>
#include <string.h>
#define RIO_BUFSIZE 8192

ssize_t rio_readn(int f, void *buf, size_t num);
ssize_t rio_writen(int f, void *buf, size_t num);

typedef struct {
	int rio_fd;        	       /*Decriptor for this internel buf*/
	int rio_cnt;		       /*Unread bytes in internal buf*/
	char *rio_bufptr;	       /*Next unread byte in internal buf*/
	char rio_buf[RIO_BUFSIZE];     /*Internal buffer*/
} rio_t;

void rio_readinitb(rio_t *r, int f);

ssize_t rio_read(rio_t *r, char *buf, size_t num);

ssize_t rio_readlineb(rio_t *r, void *buf, size_t max);

ssize_t rio_readnb(rio_t *rp, void *buf, size_t num);
