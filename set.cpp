#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 100

typedef struct SetNode {
    void* value;
    struct SetNode* next;
} SetNode;

typedef struct Set {
    SetNode * table[TABLE_SIZE];
    unsigned int size;
    unsigned int (*hash)(void*);
    int (*compare)(void*, void*);
} Set;

unsigned int intHash(void* value) {
    return (*(int*)value) % TABLE_SIZE;
}

int intCompare(void* a, void* b) {
    return (*(int*)a) - (*(int*)b);
}

void initializeSet(Set* set, unsigned int (*hash)(void*), int (*compare)(void*, void*)) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        set->table[i] = NULL;
    }
    set->hash = hash;
    set->compare = compare;
    set->size = 0;
}

// Create a new node
SetNode* createNode(void* value) { //, size_t valueSize) {
    SetNode* newNode = (SetNode*)malloc(sizeof(SetNode));
    // newNode->value = malloc(valueSize);
    // memcpy(newNode->value, value, valueSize);
    newNode->value = value;
    newNode->next = NULL;
    return newNode;
}

// Insert value into set
int insert(Set* set, void* value) { //, size_t valueSize) {
    unsigned int index = set->hash(value);

    if (set->table[index] == NULL) {
        SetNode* newNode = createNode(value); //, valueSize);
        set->table[index] = newNode;
        set->size++;
        return 1;
    } else {
        SetNode* current = set->table[index];
        while (current->next != NULL) {
            if (set->compare(current->value, value) == 0) {
                // Value already exists, no need to insert
                return 0;
            }
            current = current->next;
        }
        // Add the new node to the end of the list
        if (set->compare(current->value, value) != 0) {
            SetNode* newNode = createNode(value); //, valueSize);
            current->next = newNode;
            set->size++;
            return 1;
        }
        // otherwise, value already exists, no need to insert
        return 0;
    }
}

// Check if value exists in set
int contains(Set* set, void* value) {
    unsigned int index = set->hash(value);
    SetNode* current = set->table[index];
    while (current != NULL) {
        if (set->compare(current->value, value) == 0) {
            return 1; // Value found
        }
        current = current->next;
    }
    return 0; // Value not found
}

void * get(Set* set, void* value) {
    unsigned int index = set->hash(value);
    SetNode* current = set->table[index];
    while (current != NULL) {
        if (set->compare(current->value, value) == 0) {
            return current->value; // Value found
        }
        current = current->next;
    }
    return NULL; // Value not found
}

// Remove value from set
void remove(Set* set, void* value) {
    unsigned int index = set->hash(value);
    SetNode* current = set->table[index];
    SetNode* previous = NULL;

    while (current != NULL && set->compare(current->value, value) != 0) {
        previous = current;
        current = current->next;
    }

    if (current == NULL) {
        // Value not found
        return;
    }

    if (previous == NULL) {
        // Remove the first node
        set->table[index] = current->next;
    } else {
        // Remove the node in the middle or end
        previous->next = current->next;
    }

    // free(current->value);
    free(current);
    set->size--;
}

// Free all memory allocated for the set
void freeSet(Set* set) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        SetNode* current = set->table[i];
        while (current != NULL) {
            SetNode* temp = current;
            current = current->next;
            // free(temp->value);
            free(temp);
        }
    }
}

// Example usage
// int main() {
//     Set mySet;
//     initializeSet(&mySet, intHash, intCompare);

//     int a = 10, b = 20, c = 30;

//     insert(&mySet, &a, sizeof(int));
//     insert(&mySet, &b, sizeof(int));
//     insert(&mySet, &c, sizeof(int));
//     insert(&mySet, &c, sizeof(int));

//     // assert(mySet.size == 3);
//     printf("Set size: %d\n", mySet.size);

//     printf("Contains 20: %d\n", contains(&mySet, &b)); // Output: 1
    
//     int d = 40;
//     printf("Contains 40: %d\n", contains(&mySet, &d)); // Output: 0

//     remove(&mySet, &b);
//     printf("Contains 20 after deletion: %d\n", contains(&mySet, &b)); // Output: 0

//     printf("Set size: %d\n", mySet.size);

//     freeSet(&mySet);
//     return 0;
// }
