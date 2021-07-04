#ifndef CACTI_QUEUE_H
#define CACTI_QUEUE_H

#include <stdbool.h>
#include <stdint.h>

typedef uint_fast64_t uint64;

typedef struct queue {
    uint64 alloc_size;
    uint64 size;
    uint64 first;
    uint64 last;
    void **values;
} queue_t;

bool empty(queue_t *queue);

bool full(queue_t *queue);

queue_t *init_queue();

void *front(queue_t *queue);

void push(queue_t *queue, void *x);

void pop(queue_t *queue);

void clear(queue_t *queue);

#endif // CACTI_QUEUE_H
