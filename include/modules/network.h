// network.h
#ifndef NETWORK_H
#define NETWORK_H

#include "config.h"
#include "queue.h"
#include <stdint.h>
#include <cjson/cJSON.h>

typedef int network_Bool;

typedef struct
{
    cJSON *json;
    char ip[16];
    uint16_t port;
} network_Packet;

void network_DestroyPacket(void *data);

// Глобальные очереди (инициализируются извне)
// Очередь для отправляемых пакетов (тип network_Packet)
extern Queue *to_network;
// Очередь для принятых пакетов (тип network_Packet)
extern Queue *from_network;
// Флаг обазначающий начало приема
extern network_Bool network_receiving;
// Флаг обозначающий остановку потоков
extern volatile network_Bool network_Stop;

int network_StartThreads(uint16_t listen_port);
void network_StopThreads();

#endif // NETWORK_H
