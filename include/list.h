#pragma once

#include <stdbool.h>
#include <pthread.h>

typedef void (*freeFunction)(void*);

typedef bool (*listIterator)(void*);

typedef struct _listNode
{
        void* data;
        struct _listNode* next;
} listNode;

typedef struct
{
        int volatile length;
        int elementSize;
        listNode* head;
        listNode* tail;
        pthread_mutex_t* lock;
        pthread_mutexattr_t* mattr;
        freeFunction freeFn;
} list;

int8_t list_new(list* list, int elementSize, freeFunction freeFn);
int8_t list_destroy(list* list);
int8_t list_prepend(list* list, void* element);
int8_t list_append(list* list, void* element);
int list_size(list* list);
void list_for_each(list *list, listIterator iterator);
int8_t list_head(list* list, void* element, bool remove);
int8_t list_tail(list* list, void* element, bool remove);
int8_t list_clear(list* list);
