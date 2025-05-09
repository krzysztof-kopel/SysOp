#include "common.h"

struct queue {
    char content[MAX_MESSAGE_NUMBER][MESSAGE_LENGTH];
    int first;
    int last;
};

void add(struct queue* q, char* message) {
    q->last++;
    q->last %= MAX_MESSAGE_NUMBER;
    strcpy(q->content[q->last], message);
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
