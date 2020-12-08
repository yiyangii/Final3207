//
// Created by 310165137 on 12/1/2020.
//

#ifndef SERVER_QUEUE_H
#define SERVER_QUEUE_H

typedef struct node {
    int socket_fd;
    struct node *next;
} node;

typedef struct socket_fd_queue {
    int count;
    node *front;
    node *back;
} socket_fd_queue;

void sfq_enqueue(socket_fd_queue *q, int value);
int sfq_pop(socket_fd_queue *q);
void sfq_initialize(socket_fd_queue *q);

#endif //SERVER_QUEUE_H
