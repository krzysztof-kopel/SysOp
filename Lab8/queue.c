#include "common.h"

void add(struct queue* q, char* message) {
    strcpy(q->content[q->last], message);
    q->last++;
    q->last %= MAX_MESSAGE_NUMBER;
}

char* get(struct queue* q) {
    char* next_value = q->content[q->first];
    q->first++;
    q->first %= MAX_MESSAGE_NUMBER;
    return next_value;
}

void init_queue(struct queue* q) {
    q->first = 0;
    q->last = 0;
    for (int i = 0; i < MAX_MESSAGE_NUMBER; i++) {
        q->content[i][0] = '\0';
    }
}

int is_empty(struct queue* q) {
    return q->first == q->last;
}
