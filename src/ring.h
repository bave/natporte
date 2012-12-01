#ifndef RING_H
#define RING_H

#include <sys/uio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

#include "utils.hpp"

#define RING_OK   0
#define RING_ERR -1
#define RING_OVER  -2
#define RING_EMPTY -3

#define RING_DEFAULT_RING_SIZE 10
#define RING_DEFAULT_BUF_SIZE 65535

struct ring_buffer {
    //unsigned char buffer[RING_DEFAULT_BUF_SIZE];
    char* buffer;
    size_t size;
    uint64_t opaque;
};

struct ring_handler {
    pthread_mutex_t lock;
    pthread_cond_t  cond;
    int pop_point;
    int push_point;
    size_t ring_size;
    struct ring_buffer* rbuf;
};

static inline void ring_init(struct ring_handler* h, size_t ring_size)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&h->lock, &attr);
    pthread_cond_init(&h->cond, NULL);

    h->pop_point  = 0;
    h->push_point = 0;

    if (ring_size == 0) {
        h->ring_size = RING_DEFAULT_RING_SIZE;
        h->rbuf = (struct ring_buffer*)malloc(sizeof(struct ring_buffer)*h->ring_size);
    } else {
        h->ring_size = ring_size;
        h->rbuf = (struct ring_buffer*)malloc(sizeof(struct ring_buffer)*h->ring_size);
    }

    unsigned int i;
    for (i=0; i<h->ring_size; i++) {
        h->rbuf[i].size = 0;
        h->rbuf[i].buffer = NULL;
    }

    return;
}

static inline int ring_push(struct ring_handler* h, char* buffer, size_t size, uint64_t opaque)
{
    if (size > RING_DEFAULT_BUF_SIZE && size <= 0) {
        return RING_ERR;
    }

    pthread_mutex_lock(&h->lock);

    int next;
    next = (h->push_point+1) % (h->ring_size);
    if (next == h->pop_point) {
        pthread_mutex_unlock(&h->lock);
        return RING_OVER;
    }


    if (buffer != NULL && size != 0) {
#ifdef OPT_HOSTID
        h->rbuf[h->push_point].buffer = (char*)malloc(4096);
        //memset(h->rbuf[h->push_point].buffer, 0, 9600);
#else
        h->rbuf[h->push_point].buffer = (char*)malloc(size);
#endif
        memcpy(h->rbuf[h->push_point].buffer, buffer, size);
        h->rbuf[h->push_point].size = size;
    }
    h->rbuf[h->push_point].opaque = opaque;

    h->push_point = next;

    pthread_cond_signal(&h->cond);
    pthread_mutex_unlock(&h->lock);

    return RING_OK;
}

// struct ring_buffer* r is already expected to allcate memory.
static inline int ring_s_push(struct ring_handler* h, struct ring_buffer* r)
{
    if (r->size > RING_DEFAULT_BUF_SIZE && r->size <= 0) {
        return RING_ERR;
    }

    pthread_mutex_lock(&h->lock);

    int next;
    next = (h->push_point+1) % (h->ring_size);
    if (next == h->pop_point) {
        pthread_mutex_unlock(&h->lock);
        return RING_OVER;
    }


    h->rbuf[h->push_point].buffer = r->buffer;
    h->rbuf[h->push_point].size = r->size;
    h->rbuf[h->push_point].opaque = r->opaque;

    h->push_point = next;

    pthread_cond_signal(&h->cond);
    pthread_mutex_unlock(&h->lock);

    return RING_OK;
}

// this function must need free() for the buffer after using.
static inline int ring_pop(struct ring_handler* h, char** buffer, size_t* size, uint64_t* opaque)
{
    pthread_mutex_lock(&h->lock);

    if (h->push_point == h->pop_point) {
        *buffer = NULL; 
        *size  = 0;
        pthread_mutex_unlock(&h->lock);
        return RING_EMPTY;
    }

    *size  = h->rbuf[h->pop_point].size;
    *buffer = h->rbuf[h->pop_point].buffer;
    if (opaque != NULL) *opaque = h->rbuf[h->pop_point].opaque;

    h->rbuf[h->pop_point].buffer = NULL;
    h->rbuf[h->pop_point].size   = 0;
    h->rbuf[h->pop_point].opaque = 0;

    h->pop_point = (h->pop_point + 1) % (h->ring_size);

    pthread_mutex_unlock(&h->lock);

    return RING_OK;
}

static inline int ring_pop_condblock(struct ring_handler* h, char** buffer, size_t* size, uint64_t* opaque)
{
    pthread_mutex_lock(&h->lock);

    ring_pop_condblock_loop: 
    if (h->push_point == h->pop_point) {
        pthread_cond_wait(&h->cond, &h->lock);
        goto ring_pop_condblock_loop;
    }

    *size  = h->rbuf[h->pop_point].size;
    *buffer = h->rbuf[h->pop_point].buffer;
    if (opaque != NULL) *opaque = h->rbuf[h->pop_point].opaque;
    h->rbuf[h->pop_point].buffer = NULL;
    h->rbuf[h->pop_point].size   = 0;
    h->rbuf[h->pop_point].opaque = 0;

    h->pop_point = (h->pop_point + 1) % (h->ring_size);

    pthread_mutex_unlock(&h->lock);

    return RING_OK;
}


// struct ring_buffer* r is already expected to allcate memory.
static inline int ring_s_pop(struct ring_handler* h, struct ring_buffer* r)
{
    pthread_mutex_lock(&h->lock);

    if (h->push_point == h->pop_point) {
        r->buffer = NULL; 
        r->size = 0; 
        pthread_mutex_unlock(&h->lock);
        return RING_EMPTY;
    }

    pthread_mutex_lock(&h->lock);

    r->buffer = h->rbuf[h->pop_point].buffer;
    r->size = h->rbuf[h->pop_point].size;
    r->opaque = h->rbuf[h->pop_point].opaque;
    h->rbuf[h->pop_point].buffer = NULL;
    h->rbuf[h->pop_point].size = 0;
    h->rbuf[h->pop_point].opaque = 0;

    h->pop_point = (h->pop_point + 1) % (h->ring_size);

    pthread_mutex_unlock(&h->lock);

    return RING_OK;
}

// struct ring_buffer* r is already expected to allcate memory.
static inline int ring_s_pop_condblock(struct ring_handler* h, struct ring_buffer* r)
{
    pthread_mutex_lock(&h->lock);
    ring_s_pop_condblock_loop: 
    if (h->push_point == h->pop_point) {
        pthread_cond_wait(&h->cond, &h->lock);
        goto ring_s_pop_condblock_loop;
    }

    r->buffer = h->rbuf[h->pop_point].buffer;
    r->size = h->rbuf[h->pop_point].size;
    r->opaque = h->rbuf[h->pop_point].opaque;
    h->rbuf[h->pop_point].buffer = NULL;
    h->rbuf[h->pop_point].size = 0;
    h->rbuf[h->pop_point].opaque = 0;

    h->pop_point = (h->pop_point + 1) % (h->ring_size);

    pthread_mutex_unlock(&h->lock);

    return RING_OK;
}

#endif // RING_H
