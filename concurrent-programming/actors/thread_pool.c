#include <stdlib.h>
#include "thread_pool.h"

/* IMPORTANT:
 * Thread pool interface and functions implementations use modified code from this site
 * https://nachtimwald.com/2019/04/12/thread-pool-in-c/
 * It's author, John Schember, licenses it under MIT license:
 * https://nachtimwald.com/legal/
 * Code is modified accordingly to meet the requirements of this project.
 */

static tpool_work_t *tpool_work_create(thread_func_t func, void *arg) {
    tpool_work_t *work;

    if (func == NULL) {
        return NULL;
    }

    work = malloc(sizeof(tpool_work_t));
    work->func = func;
    work->arg = arg;
    return work;
}

static void *tpool_worker(void *arg) {
    tpool_t *tp = (tpool_t *) arg;
    tpool_work_t *work;

    while (1) {
        pthread_mutex_lock(&(tp->work_mutex));

        while (empty(tp->work_queue) && !(tp->stop)) {
            pthread_cond_wait(&(tp->work_cond), &(tp->work_mutex));
        }

        if (tp->stop) {
            break;
        }

        work = front(tp->work_queue);
        pop(tp->work_queue);
        (tp->working_count)++;
        pthread_mutex_unlock(&(tp->work_mutex));

        if (work != NULL) {
            work->func(work->arg);
            free(work);
        }

        pthread_mutex_lock(&(tp->work_mutex));
        (tp->working_count)--;
        if (!(tp->stop) && (tp->working_count) == 0 && empty(tp->work_queue)) {
            pthread_cond_signal(&(tp->working_cond));
        }
        pthread_mutex_unlock(&(tp->work_mutex));
    }

    (tp->thread_count)--;
    pthread_cond_signal(&(tp->working_cond));
    pthread_mutex_unlock(&(tp->work_mutex));
    return NULL;
}

static void tpool_join(tpool_t *tp) {
    for (unsigned int i = 0; i < tp->threads_num; ++i) {
        pthread_join(tp->threads[i], NULL);
    }
}

tpool_t *tpool_create(size_t num) {
    tpool_t *tp;
    size_t i;

    tp = malloc(sizeof(tpool_t));
    tp->threads_num = num;
    tp->thread_count = num;
    tp->working_count = 0;
    tp->stop = false;

    pthread_mutex_init(&(tp->work_mutex), NULL);
    pthread_cond_init(&(tp->work_cond), NULL);
    pthread_cond_init(&(tp->working_cond), NULL);

    tp->work_queue = init_queue();
    tp->threads = malloc(num * sizeof(pthread_t));

    for (i = 0; i < num; i++) {
        pthread_create(&(tp->threads[i]), NULL, &tpool_worker, tp);
    }

    return tp;
}

void tpool_destroy(tpool_t *tp) {
    tpool_work_t *work;

    if (tp == NULL) {
        return;
    }

    pthread_mutex_lock(&(tp->work_mutex));
    while (!empty(tp->work_queue)) {
        work = front(tp->work_queue);
        pop(tp->work_queue);
        free(work);
    }
    tp->stop = true;
    pthread_cond_broadcast(&(tp->work_cond));
    pthread_mutex_unlock(&(tp->work_mutex));

    tpool_wait(tp);
    tpool_join(tp);

    pthread_mutex_destroy(&(tp->work_mutex));
    pthread_cond_destroy(&(tp->work_cond));
    pthread_cond_destroy(&(tp->working_cond));

    clear(tp->work_queue);
    free(tp->threads);
    free(tp);
}

bool tpool_add_work(tpool_t *tp, thread_func_t func, void *arg) {
    tpool_work_t *work;

    if (tp == NULL) {
        return false;
    }

    work = tpool_work_create(func, arg);
    if (work == NULL) {
        return false;
    }

    pthread_mutex_lock(&(tp->work_mutex));
    push(tp->work_queue, work);

    pthread_cond_broadcast(&(tp->work_cond));
    pthread_mutex_unlock(&(tp->work_mutex));

    return true;
}

void tpool_wait(tpool_t *tp) {
    if (tp == NULL) {
        return;
    }

    pthread_mutex_lock(&(tp->work_mutex));
    while (1) {
        if ((!(tp->stop) && tp->working_count != 0) || (tp->stop && tp->thread_count != 0)) {
            pthread_cond_wait(&(tp->working_cond), &(tp->work_mutex));
        } else {
            break;
        }
    }
    pthread_mutex_unlock(&(tp->work_mutex));
}
