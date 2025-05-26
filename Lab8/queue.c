#include "common.h"

void init_queue(struct queue* q) {
    q->first = q->last = q->size = 0;
    for (int i = 0; i < MAX_MESSAGE_NUMBER; i++) {
        q->content[i][0] = '\0';
    }
}

int is_empty(struct queue* q) {
    return q->size == 0;
}

int is_full(struct queue* q) {
    return q->size == MAX_MESSAGE_NUMBER;
}

int add(struct queue* q, char* message) {
    if (is_full(q)) {
        return 0;
    }

    strcpy(q->content[q->last], message);

    q->last = (q->last + 1) % MAX_MESSAGE_NUMBER;
    q->size++;
    return 1;
}

char* get(struct queue* q) {
    if (is_empty(q)) {
        return NULL;
    }

    char* msg = q->content[q->first];
    q->first = (q->first + 1) % MAX_MESSAGE_NUMBER;
    q->size--;
    return msg;
}
