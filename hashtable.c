
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"

#define CAPACITY 50000 // Size of the Hash Table

unsigned long hash_function(char* str) {
    unsigned long i = 0;
    for (int j=0; str[j]; j++)
        i += str[j];
    return i % CAPACITY;
}

static CollisionList* allocate_list () {
    // Allocates memory for a Linkedlist pointer
    CollisionList* list = (CollisionList*) calloc (1, sizeof(CollisionList));
    return list;
}

static CollisionList* linkedlist_insert(CollisionList* list, hashtable_item* item) {
    // Inserts the item onto the Linked List
    if (!list) {
        CollisionList* head = allocate_list();
        head->item = item;
        head->next = NULL;
        list = head;
        return list;
    }

    else if (list->next == NULL) {
        CollisionList* node = allocate_list();
        node->item = item;
        node->next = NULL;
        list->next = node;
        return list;
    }

    CollisionList* temp = list;
    while (temp->next) {
        temp = temp->next;
    }

    CollisionList* node = allocate_list();
    node->item = item;
    node->next = NULL;
    temp->next = node;

    return list;
}


static void free_linkedlist(CollisionList* list) {
    CollisionList* temp = list;
    if (!list)
        return;
    while (list) {
        temp = list;
        list = list->next;
        free(temp->item->key);
        free(temp->item->data);
        free(temp->item);
        free(temp);
    }
}

static CollisionList** create_overflow_buckets(HashTable* table) {
    // Create the overflow buckets; an array of linkedlists
    CollisionList** buckets = (CollisionList**) calloc (table->size, sizeof(CollisionList*));
    for (int i=0; i<table->size; i++)
        buckets[i] = NULL;
    return buckets;
}

hashtable_item* create_item(char* key, char* value) {
    // Creates a pointer to a new hash table item
    hashtable_item* item = (hashtable_item*) malloc (sizeof(hashtable_item));
    item->key = (char*) calloc (strlen(key) + 1, sizeof(char));
    item->data = (char*) calloc (strlen(value) + 1, sizeof(char));

    strcpy(item->key, key);
    strcpy(item->data, value);

    return item;
}

HashTable* hashtable_create_table(int size) {
    // Creates a new HashTable
    HashTable* table = (HashTable*) malloc (sizeof(HashTable));
    table->size = size;
    table->count = 0;
    table->items = (hashtable_item**) calloc (table->size, sizeof(hashtable_item*));
    for (int i=0; i<table->size; i++)
        table->items[i] = NULL;
    table->overflow_buckets = create_overflow_buckets(table);

    return table;
}

void free_item(hashtable_item* item) {
    // Frees an item
    free(item->key);
    free(item->data);
    free(item);
}

void handle_collision(HashTable* table, unsigned long index, hashtable_item* item) {
    CollisionList* head = table->overflow_buckets[index];

    if (head == NULL) {
        // We need to create the list
        head = allocate_list();
        head->item = item;
        table->overflow_buckets[index] = head;
        return;
    }
    else {
        // Insert to the list
        table->overflow_buckets[index] = linkedlist_insert(head, item);
        return;
    }
}

void hashtable_insert(HashTable* table, char* key, char* value) {
    // Create the item
    hashtable_item* item = create_item(key, value);

    // Compute the index
    int index = hash_function(key);

    hashtable_item* current_item = table->items[index];

    if (current_item == NULL) {
        // Key does not exist.
        if (table->count == table->size) {
            // Hash Table Full
            printf("Insert Error: Hash Table is full\n");
            // Remove the create item
            free_item(item);
            return;
        }

        // Insert directly
        table->items[index] = item;
        table->count++;
    }

    else {
        // Scenario 1: We only need to update value
        if (strcmp(current_item->key, key) == 0) {
            free(table->items[index]->data);
            table->items[index]->data = (char*) calloc (strlen(value) + 1, sizeof(char));
            strcpy(table->items[index]->data, value);
            free_item(item);
            return;
        }

        else {
            // Scenario 2: Collision
            handle_collision(table, index, item);
            return;
        }
    }
}

char* hashtable_search(HashTable* table, char* key) {
    // Searches the key in the hashtable
    // and returns NULL if it doesn't exist
    int index = hash_function(key);
    hashtable_item* item = table->items[index];
    CollisionList* head = table->overflow_buckets[index];

    // Ensure that we move to items which are not NULL
    while (item != NULL) {
        if (strcmp(item->key, key) == 0)
            return item->data;
        if (head == NULL)
            return NULL;
        item = head->item;
        head = head->next;
    }
    return NULL;
}