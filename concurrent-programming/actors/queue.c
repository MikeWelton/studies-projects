#include <stdlib.h>
#include "cacti.h"
#include "queue.h"

#define INIT_SIZE 64

static void avoid_cycle(queue_t *queue) {
    uint64 old_end = queue->size - 1;
    bool cond = old_end - queue->first <= queue->last;

    uint64 start_read = cond ? queue->first : 0,
            length = cond ? old_end - queue->first + 1 : queue->last + 1;
    uint64 start_write = cond ? queue->alloc_size - length : old_end + 1;
    for (uint64 k = 0; k < length; ++k) {
        queue->values[start_write + k] = queue->values[start_read + k];
    }
    if (cond) {
        queue->first = start_write;
    } else {
        queue->last = old_end + length;
    }
}

bool empty(queue_t *queue) {
    return queue->size == 0;
}

bool full(queue_t *queue) {
    return queue->size == ACTOR_QUEUE_LIMIT;
}

queue_t *init_queue() {
    queue_t *queue = malloc(sizeof(queue_t));
    queue->size = 0;
    queue->alloc_size = INIT_SIZE;
    queue->values = malloc(INIT_SIZE * sizeof(void *));
    queue->first = 1;
    queue->last = 0;
    return queue;
}

void *front(queue_t *queue) {
    if (queue->size == 0) {
        return NULL;
    }
    return queue->values[queue->first];
}

void push(queue_t *queue, void *x) {
    if (queue->size >= (queue->alloc_size)) {
        queue->alloc_size *= 2;
        queue->values = realloc(queue->values, (queue->alloc_size) * sizeof(void *));
        if (queue->last < queue->first) {
            avoid_cycle(queue);
        }
    }
    queue->last = (queue->last + 1) % (queue->alloc_size);
    (queue->size)++;
    queue->values[queue->last] = x;
}

void pop(queue_t *queue) {
    if (queue->size == 0) {
        return;
    }
    (queue->size)--;
    queue->first = (queue->first + 1) % (queue->alloc_size);
}

void clear(queue_t *queue) {
    free(queue->values);
    free(queue);
}
