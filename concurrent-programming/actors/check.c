#include <stdlib.h>
#include <stdio.h>
#include "queue.h"


typedef struct test {
    int value;
    long num;
} test_t;

int main() {
    queue_t *queue = init_queue();
    test_t *test[17];
    for(int i = 1; i < 10; ++i) {
        test[i] = malloc(sizeof(test_t));
        test[i]->value = i;
        test[i]->num = i + 4;
        push(queue, i);
        /*if(i % 2 == 0) {
            pop(queue);
        }*/
    }

    /*while(!empty(queue)) {
        int i = front(queue);
        pop(queue);
        //printf("%d %ld\n",take->value, take->num);
    }*/

    for(int i = 1; i < 10; ++i) {
       pop(queue);
    }

    for(int i = 10; i < 28; ++i) {
        push(queue, i);
    }

    for(int i = 1; i < 10; ++i) {
        free(test[i]);
    }
    clear(queue);
}
