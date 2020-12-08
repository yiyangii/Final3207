#ifndef HASHTABLE_H
#define HASHTABLE_H

typedef struct hashtable_item hashtable_item;

// Define the Hash Table Item here
struct hashtable_item {
    char* key;
    char* data;
};

typedef struct CollisionList CollisionList;

// Define the Linkedlist here
struct CollisionList {
    hashtable_item* item;
    CollisionList* next;
};


typedef struct HashTable HashTable;

// Define the Hash Table here
struct HashTable {
    // Contains an array of pointers
    // to items
    hashtable_item** items;
    CollisionList** overflow_buckets;
    int size;
    int count;
};

void hashtable_insert(HashTable* table, char* key, char* value);
char* hashtable_search(HashTable* table, char* key);
HashTable* hashtable_create_table(int size);
#endif /* HASHTABLE_H */