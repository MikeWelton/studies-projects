#ifndef THREAD_POOL_H
#define THREAD_POOL_H

/* IMPORTANT:
 * Thread pool interface and functions implementations use modified code from this site
 * https://nachtimwald.com/2019/04/12/thread-pool-in-c/
 * It's author, John Schember, licenses it under MIT license:
 * https://nachtimwald.com/legal/
 */

#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>
#include "queue.h"

typedef void (*thread_func_t)(void *arg);

typedef struct tpool_work {
    thread_func_t func;
    void *arg;
} tpool_work_t;

typedef struct tpool {
    pthread_t *threads;
    queue_t *work_queue;
    pthread_mutex_t work_mutex;
    pthread_cond_t work_cond;
    pthread_cond_t working_cond;
    size_t working_count;
    size_t thread_count;
    size_t threads_num;
    bool stop;
} tpool_t;


tpool_t *tpool_create(size_t num);

void tpool_destroy(tpool_t *tm);

/* Adds new function to work queue. */
bool tpool_add_work(tpool_t *tm, thread_func_t func, void *arg);

/* Waits for all threads to end work. */
void tpool_wait(tpool_t *tm);

#endif // THREAD_POOL_H
