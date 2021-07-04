#include <semaphore.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>
#include "cacti.h"
#include "queue.h"
#include "thread_pool.h"

#define INIT_SIZE 64
#define unused __attribute((unused))

typedef uint_fast64_t uint64;

typedef struct actor {
    pthread_mutex_t mutex;
    void *local_mem;
    bool dead;
    bool end;
    actor_id_t id;
    role_t *role;
    queue_t *msg_queue;
} actor_t;

typedef struct actor_system {
    pthread_t sigint_thread;
    sigset_t sigset;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    tpool_t *tpool;
    bool sigint;
    actor_id_t index;
    actor_id_t size;
    actor_id_t alive;
    actor_id_t spwn_msg_count;
    actor_id_t nactors;
    actor_t **actors;
} actor_system_t;

actor_system_t asystem = {.index = 0};

_Thread_local actor_id_t act_id = 0;


/* Returns id of newly created actor. */
static actor_id_t init_actor(role_t *const role) {
    if (asystem.nactors == asystem.size) {
        (asystem.size) *= 2;
        asystem.actors = realloc(asystem.actors, asystem.size * sizeof(actor_t *));
    }
    actor_id_t i = (asystem.index)++;
    asystem.actors[i] = malloc(sizeof(actor_t));
    actor_t *actor = asystem.actors[i];

    pthread_mutex_init(&(actor->mutex), NULL);
    actor->local_mem = NULL;
    actor->dead = false;
    actor->end = false;
    actor->id = i;
    actor->role = role;
    actor->msg_queue = init_queue();

    (asystem.alive)++;
    (asystem.nactors)++;

    return actor->id;
}

/* Returns 0 if system initialization for successful, -1 otherwise. */
static int init_system(role_t *const role) {
    if (pthread_mutex_init(&(asystem.lock), NULL) != 0) {
        return -1;
    }
    if (pthread_cond_init(&(asystem.cond), NULL) != 0) {
        return -1;
    }
    asystem.tpool = tpool_create(POOL_SIZE);
    asystem.sigint = false;
    asystem.index = 0;
    asystem.size = INIT_SIZE;
    asystem.alive = 0;
    asystem.spwn_msg_count = 1;
    asystem.nactors = 0;
    asystem.actors = malloc(INIT_SIZE * sizeof(actor_t *));
    if (asystem.tpool == NULL || asystem.actors == NULL) {
        return -1;
    }
    init_actor(role);
    return 0;
}

static void destroy_actor(actor_t *actor) {
    pthread_mutex_destroy(&(actor->mutex));
    clear(actor->msg_queue);
    asystem.index = 0;
}

static void destroy_system() {
    for (long i = 0; i < asystem.nactors; ++i) {
        destroy_actor(asystem.actors[i]);
        free(asystem.actors[i]);
    }
    pthread_mutex_destroy(&(asystem.lock));
    pthread_cond_destroy(&(asystem.cond));
    tpool_destroy(asystem.tpool);
    free(asystem.actors);
}


static void sigint_handler() {
    pthread_mutex_lock(&(asystem.lock));
    if (!asystem.sigint) {
        asystem.sigint = true;
        pthread_mutex_unlock(&(asystem.lock));
        tpool_wait(asystem.tpool);
        asystem.alive = 0;
        pthread_cond_signal(&(asystem.cond));
        destroy_system();
    } else {
        pthread_mutex_unlock(&(asystem.lock));
    }
}

static void *wait_for_sigint(unused void *arg) {
    pthread_mutex_lock(&(asystem.lock));
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&(asystem.sigset), SIGINT);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
    signal(SIGINT, sigint_handler);
    pthread_mutex_unlock(&(asystem.lock));

    int sig;
    sigwait(&(asystem.sigset), &sig);
    sigint_handler();

    return 0;
}


static void manage_spawn_msg(actor_t *actor, role_t *role) {
    pthread_mutex_unlock(&(actor->mutex));
    pthread_mutex_lock(&(asystem.lock));
    pthread_mutex_lock(&(actor->mutex));
    actor_id_t spwnd_act_id = init_actor(role);
    pthread_mutex_unlock(&(actor->mutex));
    pthread_mutex_unlock(&(asystem.lock));

    message_t message = {
            .message_type = MSG_HELLO,
            .nbytes = sizeof(actor_id_t),
            .data = (void *) (actor->id)
    };
    send_message(spwnd_act_id, message);
}

static void manage_godie_msg(actor_t *actor) {
    // zwolnienie czegoś itd.
    actor->dead = true;
    pthread_mutex_unlock(&(actor->mutex));
}

static void manage_hello_msg(actor_t *actor, message_t message) {
    message_type_t type = message.message_type;
    pthread_mutex_unlock(&(actor->mutex));
    actor->role->prompts[type](&(actor->local_mem), message.nbytes, message.data);
}

static void manage_other_msg(actor_t *actor, message_t message) {
    message_type_t type = message.message_type;
    pthread_mutex_unlock(&(actor->mutex));
    actor->role->prompts[type](&(actor->local_mem), message.nbytes, message.data);
}

static void dequeue_msg(actor_t *actor) {
    queue_t *queue = actor->msg_queue;
    message_t *msg = (message_t *) front(queue);
    pop(queue);
    if (msg->message_type == MSG_SPAWN) {
        manage_spawn_msg(actor, msg->data);
    } else if (msg->message_type == MSG_GODIE) {
        manage_godie_msg(actor);
    } else if (msg->message_type == MSG_HELLO) {
        manage_hello_msg(actor, *msg);
    } else {
        manage_other_msg(actor, *msg);
    }

    free(msg);
}

static void execute(actor_t *actor) {
    pthread_mutex_lock(&(actor->mutex));
    act_id = actor->id;
    dequeue_msg(actor);

    pthread_mutex_lock(&(asystem.lock));
    pthread_mutex_lock(&(actor->mutex));

    if (empty(actor->msg_queue) && actor->dead && !(actor->end)) {
        actor->end = true;
        (asystem.alive)--;
        if (asystem.alive == 0) {
            pthread_cond_signal(&(asystem.cond));
        }
    }
    pthread_mutex_unlock(&(actor->mutex));
    pthread_mutex_unlock(&(asystem.lock));
}


static void enqueue_msg(actor_t *actor, message_t message) {
    message_t *msg = malloc(sizeof(message_t));
    *msg = message;
    push(actor->msg_queue, msg);
}

static bool correct_message(actor_t *actor, message_t message) {
    message_type_t msg_type = message.message_type;
    return msg_type == MSG_SPAWN || msg_type == MSG_GODIE || msg_type == MSG_HELLO ||
           (0 < msg_type && (uint64) msg_type < actor->role->nprompts);
}

/* Returns
 * 0 in case of success,
 * -4 if limit of actors was reached,
 * -5 if message type is incorrect. */
static int manage_message(actor_t *actor, message_t message) {
    if (correct_message(actor, message)) {
        if (message.message_type == MSG_SPAWN &&
            asystem.spwn_msg_count >= CAST_LIMIT) { // limit of actors was reached
            return -4;
        }
        (asystem.spwn_msg_count)++;
        enqueue_msg(actor, message);
        tpool_add_work(asystem.tpool, (thread_func_t) &execute, actor);
    } else { // message type is incorrect
        return -5;
    }
    return 0;
}


/* Returns 0 in case of success or -1 otherwise. */
int actor_system_create(actor_id_t *actor, role_t *const role) {
    if (init_system(role) == -1) {
        return -1;
    }

    pthread_mutex_lock(&(asystem.lock));
    pthread_mutex_lock(&(asystem.actors[0]->mutex));
    message_t msg_hello_first = {.message_type = MSG_HELLO, .nbytes = sizeof(actor_id_t), .data = 0};

    sigfillset(&(asystem.sigset));
    pthread_sigmask(SIG_BLOCK, &(asystem.sigset), NULL);
    pthread_create(&(asystem.sigint_thread), NULL, &wait_for_sigint, 0);
    pthread_detach(asystem.sigint_thread);
    pthread_mutex_unlock(&(asystem.actors[0]->mutex));
    pthread_mutex_unlock(&(asystem.lock));

    send_message(0, msg_hello_first);
    *actor = 0;

    return 0;
}

void actor_system_join(actor_id_t actor) {
    pthread_mutex_lock(&(asystem.lock));
    if (0 <= actor && actor < asystem.index && !(asystem.sigint)) {
        actor_id_t alive = asystem.alive;

        while (alive != 0) {
            pthread_cond_wait(&(asystem.cond), &(asystem.lock));
            alive = asystem.alive;
        }

        if (!(asystem.sigint)) {
            asystem.sigint = true;
            pthread_kill(asystem.sigint_thread, SIGINT);

            pthread_mutex_unlock(&(asystem.lock));
            tpool_wait(asystem.tpool);
            destroy_system();
        }
    } else {
        pthread_mutex_unlock(&(asystem.lock));
    }
}

/* Returns
 * 0 in case of success,
 * -1 if recipient is not receiving messages,
 * -2 if actor with given id doesn't exist,
 * -3 if actor's queue is full,
 * -4 if limit of actors was reached,
 * -5 if message type is incorrect. */
int send_message(actor_id_t actor, message_t message) {
    int ret;
    if (actor >= asystem.index) { // actor doesn't exist
        return -2;
    }
    pthread_mutex_lock(&(asystem.lock));
    actor_t *act = asystem.actors[actor];
    pthread_mutex_lock(&(act->mutex));

    if (act->dead || asystem.sigint) { // actor doesn't receive messages
        ret = -1;
    } else if (full(act->msg_queue)) { // actor queue full
        ret = -3;
    } else {
        ret = manage_message(act, message);
    }
    pthread_mutex_unlock(&(act->mutex));
    pthread_mutex_unlock(&(asystem.lock));

    return ret;
}

actor_id_t actor_id_self() {
    return act_id;
}
