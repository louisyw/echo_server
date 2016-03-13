/*标准库中的read和write函数的缺陷
  读时遇到EOF，准备读一个文件，该文件从当前位置开始只含有20字节，而我们以50
  字节的片进行读取。这样一来，下一个read返回的不足值为20，此后的read将通过返  回不足值0来发出EOF信号
  
  从终端读文本行   打开文件是与终端相关联的，那么每个read函数将一次传送一个   文本行，返回的不足值等于文本行的大小
  Robust IO
 */
#include "rio.h"

/*直接从文件中读取到用户缓冲区，此函数与read的区别为:1.允许中断的系统调用
 *，2.实现了连续读，但还是会进行多次系统调用
 */
ssize_t rio_readn(int fd, void *usrbuf, size_t n)
{
	size_t nleft = n;
	ssize_t nread;
	char *bufp = (char *)usrbuf;

	while(nleft > 0 ){
		if((nread = read(fd, bufp, nleft)) < 0){ /*error*/
			if (errno == EINTR)
				nread = 0;
			else 
				return -1;
		}
		else if (nread == 0)  //EOF
			break;
		nleft -= nread;
		bufp += nread;
	}
	return (n - nleft);
}

ssize_t rio_writen(int fd, void *usrbuf, size_t n)
{
	size_t nleft = n;
	ssize_t nwritten;
	char *bufp = (char *)usrbuf;

	while(nleft > 0){
		if ((nwritten = write(fd, bufp, nleft)) <= 0){
			if (errno = EINTR)
				nwritten = 0;
			else 
				return -1;
		}
		nleft -= nwritten;
		bufp += nwritten;
	}
	return n;
}

void rio_readinitb(rio_t *rp , int fd)
{
	rp->rio_fd = fd;
	rp->rio_cnt = 0;
	rp->rio_bufptr = rp->rio_buf;
}

/*从缓冲区中读取数据放入到用户的buf中去
 *读取数据的过程为:文件->内部缓冲区->用户缓冲区，值得注意的是这个函数解放了
 *用户，使得用户可以随时从内部缓冲区读东西，而不用在进行read系统调用
 */
ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)  
{
	int cnt;

	/*copy the data into internal buf*/
	while(rp->rio_cnt <= 0 ){     //替换成if可行????
		rp->rio_cnt = read (rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
		if (rp->rio_cnt < 0){
			if (errno != EINTR)
				return -1;
		}else if(rp->rio_cnt == 0){     /*EOF，此处的情形为读到0字节*/
			return 0;
		}else{
			rp->rio_bufptr = rp->rio_buf;    //reset buffer ptr
		}
	}

	/*Copy min(n, rp->rio_cnt) bytes from internal buf to user buf*/
	cnt = n;
	if (rp->rio_cnt < n)
		cnt = rp->rio_cnt;
	memcpy(usrbuf, rp->rio_bufptr, cnt);
	rp->rio_bufptr += cnt;
	rp->rio_cnt -= cnt;
	return cnt;
}

/*一次读取一行数据,实现了从文件rp读出一行(包括结尾的换行符)，将它拷贝到
 *存储器位置usrbuf中，并且用空字符来结束这个文本行，最多读取maxlen-1个字
 *节，余下的一个字符保留给结尾的空字符。超过maxlen-1的文本行将被截断，并
 *用空字符结束*/
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen)
{
	int n, rc;
	char c, *bufp = (char *)usrbuf;

	
	for (n =1; n < maxlen; n++){   /*最多读取maxlen-1个字符*/
		/*逐字符读取，直到遇到换行符，则表示一行结束*/
		if ((rc = rio_read(rp, &c, 1)) == 1){
			*bufp++ = c;
			if (c == '\n')
				break;
		}else if (rc == 0){   //EOF
			if (n == 1)      //EOF, no data read
				return 0;   
			else             //EOF, read some data
				break;
		}else 
			return -1;
	}

	*bufp = 0;
	return n;
}

ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n)
{
	size_t nleft = n;
	ssize_t nread;
	char *bufp = (char *)usrbuf;

	while(nleft > 0){
		if ((nread = rio_read(rp, bufp, nleft)) < 0){
			if (errno == EINTR)
				nread = 0;
			else 
				return -1;
		}else if ( nread == 0 )
			break;
		nleft -= nread;
		bufp += nread;
	}
	return (n - nleft);
}
