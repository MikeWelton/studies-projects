#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "cacti.h"

#define unused __attribute((unused))
#define MSG_CALC (message_type_t)1

typedef uint_fast64_t uint64;

typedef struct cell {
    uint64 value;
    uint64 milis;
} cell_t;

uint64 grows = 0, gcols = 0;
cell_t **gmatrix;
uint64 curr_act_i = 0;
uint64 curr_row_i = 0;
actor_id_t *actors_ids;
role_t role;
uint64 result = 0;

cell_t **alloc_matrix(uint64 rows, uint64 cols) {
    cell_t **matrix = (cell_t **) malloc(rows * sizeof(cell_t *));
    for (uint64 i = 0; i < rows; ++i) {
        matrix[i] = (cell_t *) malloc(cols * sizeof(cell_t));
    }

    return matrix;
}

void clear_matrix(cell_t **matrix, uint64 rows) {
    for (uint64 i = 0; i < rows; ++i) {
        free(matrix[i]);
    }
    free(matrix);
}

cell_t **read_input(uint64 *rows, uint64 *cols) {
    cell_t **matrix;

    scanf("%ld", rows);
    scanf("%ld", cols);
    matrix = alloc_matrix(*rows, *cols);
    for (uint64 i = 0; i < *rows; ++i) {
        for (uint64 j = 0; j < *cols; ++j) {
            uint64 value, milis;
            scanf("%ld", &value);
            scanf("%ld", &milis);
            matrix[i][j].value = value;
            matrix[i][j].milis = milis;
        }
    }

    return matrix;
}

message_t calc_msg(uint64 *data) {
    message_t msg_calc = {.message_type = MSG_CALC, .nbytes = sizeof(uint64), .data = data};
    return msg_calc;
}

message_t godie_msg() {
    message_t msg_godie = {.message_type = MSG_GODIE, .nbytes = 0, .data = NULL};
    return msg_godie;
}

message_t spwn_msg() {
    message_t msg_spawn = {.message_type = MSG_SPAWN, .nbytes = sizeof(role), .data = &role};
    return msg_spawn;
}

void calculate(uint64 *stateptr, unused size_t nbytes, uint64 *data) {
    uint64 micros = 1000 * (gmatrix[curr_row_i][*stateptr].milis);
    usleep(micros);
    (*data) += (gmatrix[curr_row_i][*stateptr].value);
    curr_act_i++;

    if (curr_act_i < gcols) {
        send_message(actors_ids[curr_act_i], calc_msg(data));
        if (curr_row_i == grows - 1) {
            send_message(actor_id_self(), godie_msg());
        }
    } else {
        printf("%ld\n", result);
        *data = 0;
        curr_act_i = 0;
        curr_row_i++;
        if (curr_row_i < grows) {
            send_message(actors_ids[curr_act_i], calc_msg(data));
        } else {
            send_message(actor_id_self(), godie_msg());
        }
    }
}

void get_id(uint64 *stateptr, unused size_t nbytes, unused void *data) {
    actors_ids[curr_act_i] = actor_id_self();
    *stateptr = curr_act_i;
    curr_act_i++;

    if (curr_act_i < gcols) {
        send_message(actor_id_self(), spwn_msg());
    } else {
        curr_act_i = 0;
        send_message(actors_ids[0], calc_msg(&result));
    }
}

int main() {
    gmatrix = read_input(&grows, &gcols);
    actors_ids = malloc(gcols * sizeof(actor_id_t));

    act_t p[2] = {(act_t) &get_id, (act_t) &calculate};
    role.nprompts = 2;
    role.prompts = p;

    actor_id_t first_id;
    actor_system_create(&first_id, &role);

    actor_system_join(first_id);
    clear_matrix(gmatrix, grows);
    free(actors_ids);
    return 0;
}
