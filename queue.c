
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

void sfq_initialize(socket_fd_queue *q) {
    q->count = 0;
    q->front = NULL;
    q->back = NULL;
}

int isempty(socket_fd_queue *q) {
    if (q->count > 0) {
        return 1;
    }
    return 0;
}

void sfq_enqueue(socket_fd_queue *q, int value) {
    node *tmp;
    tmp = (node*)malloc(sizeof(node));
    tmp->socket_fd = value;
    tmp->next = NULL;
    if (isempty(q) == 1) {
        q->back->next = tmp;
        q->back = tmp;
    } else {
        q->front = tmp;
        q->back = tmp;
        tmp->next = NULL;
    }
    q->count++;
}

int sfq_pop(socket_fd_queue *q)
{
    node *tmp;
    int n = q->front->socket_fd;
    tmp = q->front;
    q->front = q->front->next;
    q->count--;
    free(tmp);
    return (n);
}
