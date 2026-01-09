// queue.c
#include "queue.h"
#include <stdio.h>

Queue *queue_Init(DataFreeFunc free_func)
{
    Queue *q = calloc(1, sizeof(Queue));
    if (!q)
        return NULL;

    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    q->data_free = free_func;
    pthread_mutex_init(&q->mutex, NULL);

    return q;
}

void queue_Destroy(Queue *q)
{
    if (!q)
        return;

    queue_Clear(q);
    pthread_mutex_destroy(&q->mutex);
}

int queue_Push(Queue *q, void *data)
{
    if (!q)
        return -1;

    QueueNode *new_node = malloc(sizeof(QueueNode));
    if (!new_node)
        return -2;

    new_node->data = data;
    new_node->next = NULL;

    pthread_mutex_lock(&q->mutex);

    if (q->tail)
        q->tail->next = new_node;
    else
        q->head = new_node;
    q->tail = new_node;
    q->size++;

    pthread_mutex_unlock(&q->mutex);
    return 0; // Успех
}

void *queue_Pop(Queue *q)
{
    if (!q || !q->head)
        return NULL;

    pthread_mutex_lock(&q->mutex);

    QueueNode *temp = q->head;
    void *data = temp->data;
    q->head = q->head->next;

    if (!q->head)
        q->tail = NULL;

    q->size--;
    free(temp);

    pthread_mutex_unlock(&q->mutex);
    return data;
}

void *queue_Peek(Queue *q)
{
    if (!q || !q->head)
        return NULL;

    pthread_mutex_lock(&q->mutex);
    void *data = q->head->data;
    pthread_mutex_unlock(&q->mutex);

    return data;
}

void queue_Clear(Queue *q)
{
    if (!q)
        return;

    pthread_mutex_lock(&q->mutex);

    QueueNode *cur = q->head;
    while (cur)
    {
        QueueNode *temp = cur;
        cur = cur->next;

        if (q->data_free && temp->data != NULL)
            q->data_free(temp->data);

        free(temp);
    }

    q->head = NULL;
    q->tail = NULL;
    q->size = 0;

    pthread_mutex_unlock(&q->mutex);
}
