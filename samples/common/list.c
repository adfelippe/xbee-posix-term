#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "list.h"


int8_t list_new(list* list, int elementSize, freeFunction freeFn)
{
        if (elementSize <= 0) goto error;

        list->length = 0;
        list->elementSize = elementSize;
        list->head = list->tail = NULL;
        list->freeFn = freeFn;

        list->mattr = calloc(1, sizeof(pthread_mutexattr_t));
        if (list->mattr == NULL) goto error;
        if (pthread_mutexattr_init(list->mattr) != 0) goto error;
        if (pthread_mutexattr_settype(list->mattr, PTHREAD_MUTEX_ERRORCHECK) != 0) goto error;

        list->lock = calloc(1, sizeof(pthread_mutex_t));
        if (list->lock == NULL) goto error;
        if (pthread_mutex_init(list->lock, list->mattr) != 0) goto error;

        return 0;

 error:
        perror("list_new");
        return -1;
}

int8_t list_destroy(list* list)
{
        if (pthread_mutex_lock(list->lock) != 0) goto error;

        listNode* current;

        while (list->head != NULL)
        {
                current = list->head;
                list->head = current->next;

                if (list->freeFn)
                {
                        list->freeFn(current->data);
                }
                else
                {
                        free(current->data);
                }

                free(current);
        }

        if (pthread_mutex_unlock(list->lock) != 0) goto error;
        if (pthread_mutexattr_destroy(list->mattr) != 0) goto error;
        if (pthread_mutex_destroy(list->lock) != 0) goto error;

        return 0;

 error:
        perror("list_destroy");
        pthread_mutex_unlock(list->lock);
        return -1;
}

int8_t list_prepend(list* list, void* element)
{
        if (pthread_mutex_lock(list->lock) != 0) goto error;

        listNode* node = calloc(1, sizeof(listNode));
        if (node == NULL) goto error;

        node->data = calloc(1, list->elementSize);
        if (node->data == NULL) goto error;

        memcpy(node->data, element, list->elementSize);

        node->next = list->head;
        list->head = node;

        if (list->tail == NULL)
        {
                list->tail = list->head;
        }

        list->length++;

        if (pthread_mutex_unlock(list->lock) != 0) goto error;

        return 0;

 error:
        perror("list_prepend");
        pthread_mutex_unlock(list->lock);
        return -1;
}

int8_t list_append(list* list, void* element)
{
        if (pthread_mutex_lock(list->lock) != 0) goto error;

        listNode* node = calloc(1, sizeof(listNode));
        if (node == NULL) goto error;

        node->data = calloc(1, list->elementSize);
        if (node->data == NULL) goto error;

        node->next = NULL;
        memcpy(node->data, element, list->elementSize);

        if (list->length == 0)
        {
                list->head = list->tail = node;
        }
        else
        {
                list->tail->next = node;
                list->tail = node;
        }

        list->length++;

        if (pthread_mutex_unlock(list->lock) != 0) goto error;

        return 0;

 error:
        perror("list_append");
        pthread_mutex_unlock(list->lock);
        return -1;
}

void list_for_each(list* list, listIterator iterator)
{
        if (pthread_mutex_lock(list->lock) != 0) goto error;

        assert(iterator != NULL);

        listNode* node = list->head;
        bool result = true;

        while (node != NULL && result)
        {
                result = iterator(node->data);
                node = node->next;
        }

        if (pthread_mutex_unlock(list->lock) != 0) goto error;
        return;

 error:
        perror("list_for_each");
        pthread_mutex_unlock(list->lock);
}

int8_t list_head(list* list, void* element, bool remove)
{
        if (pthread_mutex_lock(list->lock) != 0) goto error;

        if (list->head == NULL) goto error;

        listNode* node = list->head;
        memcpy(element, node->data, list->elementSize);

        if (remove)
        {
                list->head = node->next;
                list->length--;

                if (list->freeFn)
                {
                        list->freeFn(node->data);
                }
                else
                {
                        free(node->data);
                }

                free(node);
        }

        if (pthread_mutex_unlock(list->lock)) goto error;
        return 0;

 error:
        perror("list_head");
        pthread_mutex_unlock(list->lock);
        return -1;
}

int8_t list_tail(list* list, void* element, bool remove)
{
        if (pthread_mutex_lock(list->lock) != 0) goto error;

        if (list->tail == NULL) goto error;

        memcpy(element, list->tail->data, list->elementSize);

        if (remove)
        {
                if (list->head == list->tail)
                {
                        list->tail = NULL;

                        if (list->freeFn)
                        {
                                list->freeFn(list->head->data);
                        }
                        else
                        {
                                free(list->head->data);
                        }

                        free(list->head);
                        list->head = NULL;
                        list->length--;
                }
                else
                {
                        listNode* current = list->head;

                        while (current->next != list->tail)
                        {
                                current = current->next;
                        }

                        list->tail = current;

                        if (list->freeFn)
                        {
                                list->freeFn(current->next->data);
                        }
                        else
                        {
                                free(current->next->data);
                        }

                        free(current->next);
                        current->next = NULL;
                        list->length--;
                }
        }

        if (pthread_mutex_unlock(list->lock) != 0) goto error;

        return 0;

 error:
        perror("list_tail");
        pthread_mutex_unlock(list->lock);
        return -1;
}

int8_t list_clear(list* list)
{
        if (pthread_mutex_lock(list->lock) != 0) goto error;

        listNode* current;

        while (list->head != NULL)
        {
                current = list->head;
                list->head = current->next;

                if (list->freeFn)
                {
                        list->freeFn(current->data);
                }
                else
                {
                        free(current->data);
                }

                free(current);
                list->length--;
        }

        if (pthread_mutex_unlock(list->lock) != 0) goto error;

        return 0;

 error:
        perror("list_clear");
        pthread_mutex_unlock(list->lock);
        return -1;
}

int list_size(list* list)
{
        return list->length;
}
