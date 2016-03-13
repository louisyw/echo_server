#include "sbuf.h"
#include <stdlib.h>
#include <stdio.h>

void sbuf_init(sbuf_t *sp, int length)
{
	int err;

	sp->buf = calloc(length, sizeof(itemType));

	if (sp->buf == NULL){
		fprintf(stderr, "calloc error");
		exit(EXIT_FAILURE);
	}

	sp->n = length;

	sp->front = sp->rear = 0;

	err = sem_init(&sp->mutex, 0, 1);
	if (err != 0){
		fprintf(stderr, "sem_init mutex error\n");
		exit(EXIT_FAILURE);
	}

	err = sem_init(&sp->slots, 0, length);
	if (err != 0){
		fprintf(stderr, "sem_init slots error\n");
		exit(EXIT_FAILURE);
	}

	err = sem_init(&sp->items, 0, 0);
	if (err != 0){
		fprintf(stderr, "sem_init items error\n");
		exit(EXIT_FAILURE);
	}

}

void sbuf_deinit(sbuf_t *sp)
{
	if (sp->buf != NULL){
		free(sp->buf);
		sp->buf = NULL;
	}
}

void sbuf_insert(sbuf_t *sp, itemType item){
	int err;

	err = sem_wait(&sp->slots);
	if (err != 0){
		fprintf(stderr, "sem_wait slots error\n");
		exit(EXIT_FAILURE);
	}

	err = sem_wait(&sp->mutex);
	if (err != 0){
		fprintf(stderr, "sem_wait mutex errror\n");
		exit(EXIT_FAILURE);
	}

	sp->buf[sp->rear] = item;
	sp->rear = (++sp->rear) % sp->n;

	err = sem_post(&sp->mutex);
	if (err != 0){
		fprintf(stderr, "sem_post mutex error\n");
		exit(EXIT_FAILURE);
	}

	err = sem_post(&sp->items);
	if (err != 0){
		fprintf(stderr, "sem_post items error\n");
		exit(EXIT_FAILURE);
	}

}

itemType sbuf_remove(sbuf_t *sp){
	int err;
	itemType item;

	err = sem_wait(&sp->items);
	if (err != 0){
		fprintf(stderr, "sem_wait itemss error\n");
		exit(EXIT_FAILURE);
	}

	err = sem_wait(&sp->mutex);
	if (err != 0){
		fprintf(stderr, "sem_wait mutex errror\n");
		exit(EXIT_FAILURE);
	}

	item = sp->buf[sp->front];
	sp->front = (++sp->front) % sp->n;

	err = sem_post(&sp->mutex);
	if (err != 0){
		fprintf(stderr, "sem_post mutex error\n");
		exit(EXIT_FAILURE);
	}

	err = sem_post(&sp->slots);
	if (err != 0){
		fprintf(stderr, "sem_post slots error\n");
		exit(EXIT_FAILURE);
	}
	
	return item;
}
