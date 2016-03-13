#ifndef SBUF_H_
#define SBUF_H_
#include <semaphore.h>

typedef int itemType;
typedef struct{
	itemType *buf;
	int n;
	int front;
	int rear;
	sem_t mutex;                //protect the access to buf
	sem_t slots;                //counts the slot available
	sem_t items;                //count the items insert
}sbuf_t;


void sbuf_init(sbuf_t *sp, int n);
void sbuf_insert(sbuf_t *sp, itemType item);
int sbuf_remove(sbuf_t *sp);
void sbuf_deinit(sbuf_t *sp);
#endif
