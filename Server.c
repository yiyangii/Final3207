//
// Created by 310165137 on 12/1/2020.
//
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <string.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <ctype.h>
#include <getopt.h>
#include <pthread.h>
#include <dirent.h>
#include <semaphore.h>
#include "queue.h"
#include "hashtable.h"
#include "Server.h"

#define DEFAULT_DICTIONARY "dictionary.txt"
#define DEFAULT_PORT 8080
#define DEFAULT_NUM_THREADS 4
#define HASH_TABLE_CAPACITY 50000 

struct socket_fd_queue fdqueue;
HashTable* ht;
sem_t semaphore_fd;
pthread_mutex_t bin_sem_queue = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t bin_sem_fileOp = PTHREAD_MUTEX_INITIALIZER;


void server_Update_Log_File(char*fileName, char *word, char *response)
{
    FILE *fp = NULL;
    fp = fopen(fileName, "a");
    if (fp)
    {
        fprintf(fp,"%s \t %s ", word, response);
    }
    fclose(fp);
}

void *server_thread_worker(void* param) {
    while (1) {

        sem_wait(&semaphore_fd);

        pthread_mutex_lock(&bin_sem_queue);
        int comm_fd = sfq_pop(&fdqueue); // Critical Section as data structure Q is modified
        pthread_mutex_unlock(&bin_sem_queue);

        uint8_t buff[1024];
        while (recv(comm_fd, buff, 1024, 0) != 0)
        {
            buff[strcspn(buff, "\r\n")] = 0;
            printf("Client Msg : %s\n", buff);
            if (hashtable_search(ht, buff)) {
		Printf("Checking status..........");
                send(comm_fd, "Status: OK", strlen("Status: OK"), 0);
                pthread_mutex_lock(&bin_sem_fileOp);
                server_Update_Log_File("log.txt", buff, "Status: OK");
                pthread_mutex_unlock(&bin_sem_fileOp);
            }
            else
            {
		printf("Checking status..........");
                send(comm_fd, "Status: MISSPELLED", strlen("Status: MISSPELLED"), 0);
                pthread_mutex_lock(&bin_sem_fileOp);
                server_Update_Log_File("log.txt", buff, "Status: MISSPELLED");
                pthread_mutex_unlock(&bin_sem_fileOp);
            }
        }

        close(comm_fd);

    }
}

void server_Load_Dictionary(char *dictfilename)
{
    FILE *fp = NULL;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int result;

    fp = fopen(dictfilename, "r");
    if (fp)
    {
        while ((read = getline(&line, &len, fp)) != -1)
        {
            line[strcspn(line, "\r\n")] = 0;
            hashtable_insert(ht, line, line);
        }

        fclose(fp);
        if (line)
        {
            free(line);
        }
    }
}

int main(int argc, char *argv[])
{
    unsigned short PORT = DEFAULT_PORT;
    int num_threads = DEFAULT_NUM_THREADS;
    char *dict_file_name = DEFAULT_DICTIONARY;

    if (argc > 1 && argc < 4)
    {
        printf("Incorrect number of parameters \n");
        printf("Usage: %s <dictionary_file_name> <Port> <num_of_threads>\n",argv[0]);
        exit(0);
    }

    if (argc == 4)
    {
        dict_file_name = argv[1];
        PORT = atoi(argv[2]);
        num_threads = atoi(argv[3]);
    }

    sem_init(&semaphore_fd, 0, 0);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));


    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    // Setup socket and check listen function for error
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
        perror("Listen failed");
        exit(0);
    }

  
    int enable = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

    bind(listen_fd, (struct sockaddr *) &serv_addr, sizeof serv_addr);
    listen(listen_fd, 500);

 
    sfq_initialize(&fdqueue);

    ht = hashtable_create_table(HASH_TABLE_CAPACITY);

    server_Load_Dictionary(dict_file_name);

    pthread_t workers[num_threads];
    for (size_t i = 0; i < num_threads; i++)
    {
        pthread_create(&workers[i], NULL, &server_thread_worker, NULL);
    }

    printf("Server Ready for Operation\n");
    while (1)
    {
        // Accept connection
        int comm_fd = accept(listen_fd, NULL, NULL);

        /* Enqeue the new Fd to the queue */
        pthread_mutex_lock(&bin_sem_queue);
        sfq_enqueue(&fdqueue, comm_fd);
        pthread_mutex_unlock(&bin_sem_queue);

        /* Signal all the threads waiting on the queue*/
        sem_post(&semaphore_fd);
    }
}