#include <stdint.h>
#include <stdio.h>
#include "cacti.h"

#define unused __attribute((unused))

typedef uint_fast64_t uint64;

typedef struct data {
    uint64 fact;
    uint64 k;
    uint64 n;
} data_t;


role_t role;
data_t data;


void factorial(void **stateptr, size_t nbytes, data_t *d);

uint64 read_input() {
    uint64 input = 0;
    scanf("%ld", &input);
    return input;
}

void print_output(uint64 result) {
    printf("%ld\n", result);
}

message_t godie_msg() {
    message_t msg_godie = {.message_type = MSG_GODIE, .nbytes = 0, .data = NULL};
    return msg_godie;
}

void call_next() {
    message_t msg_spawn = {.message_type = MSG_SPAWN, .nbytes = sizeof(role), .data = &role};
    send_message(actor_id_self(), msg_spawn);
    send_message(actor_id_self(), godie_msg());
}

void factorial(unused void **stateptr, unused size_t nbytes, unused data_t *d) {
    if (data.fact == 0) {
        data.fact = 1;
    } else {
        (data.k)++;
        (data.fact) *= (data.k);
    }

    if (data.k == data.n) {
        print_output(data.fact);
        send_message(actor_id_self(), godie_msg());
        return;
    }
    call_next();
}

int main() {
    uint64 n = read_input();
    data_t d = {.fact = 0, .k = 0, .n = n};
    data = d;

    actor_id_t actor_id = 0;
    act_t p[1] = {(act_t) &factorial};
    role_t r = {.nprompts = 1, .prompts = p};
    role = r;

    actor_system_create(&actor_id, &r);

    actor_system_join(actor_id);

    return 0;
}