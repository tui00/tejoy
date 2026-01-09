// queue.h
#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include <pthread.h>
#include <cJSON.h>

// Узел очереди
typedef struct QueueNode
{
    void *data;
    struct QueueNode *next;
} QueueNode;

typedef void (*DataFreeFunc)(void *data);

// Структура очереди
typedef struct
{
    QueueNode *head;        // Голова (откуда извлекаем)
    QueueNode *tail;        // Хвост (куда добавляем)
    size_t size;            // Текущий размер
    pthread_mutex_t mutex;  // Мьютекс для синхронизации
    DataFreeFunc data_free; // Функция для освобождения данных
} Queue;

// Инициализация
Queue *queue_Init(DataFreeFunc free_func);

// Освобождение памяти
void queue_Destroy(Queue *q);

// Добавление элемента
int queue_Push(Queue *q, void *data);

// Извлечение элемента
void *queue_Pop(Queue *q);

// Просмотр элемента
void *queue_Peek(Queue *q);

// Очистка всех элементов
void queue_Clear(Queue *q);

#endif // QUEUE_H
