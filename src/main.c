// main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include <unistd.h>

#include <cJSON.h>

#include "modules/network.h"
#include "modules/packets.h"
#include "sed.h"
#include "queue.h"
#include "base64.h"

#include "config.h"

Queue *to_network;
Queue *from_network;

int main()
{
    to_network = queue_Init(network_DestroyPacket);
    from_network = queue_Init(network_DestroyPacket);

    network_StartThreads(43211);

    sed_Init();
    sed_GENERATE_KEYS(a_sec, a_pub);
    sed_GENERATE_KEYS(b_sec, b_pub);

    {
        cJSON *data = cJSON_CreateObject();
        cJSON_AddStringToObject(data, "test?", "yes");
        cJSON *packet = build_packet(0, a_pub, data, a_sec, b_pub);
        char *text = cJSON_Print(data);
        cJSON_Delete(data);
        printf("%s\n", text);
        free(text);

        network_Packet *net_packet = malloc(sizeof(network_Packet));
        snprintf(net_packet->ip, 16, "192.168.0.84");
        net_packet->port = 54321;
        net_packet->json = packet;
        queue_Push(to_network, net_packet);
    }
    sleep(2);
    {
        network_Packet *net_packet = queue_Pop(from_network);
        if (!net_packet)
            printf("No!");
        else
        {
            cJSON *packet;
            parse_packet(net_packet->json, NULL, NULL, &packet, b_sec);
            char *text = cJSON_Print(packet);
            cJSON_Delete(packet);
            printf("%s\n", text);
            free(text);

            network_DestroyPacket(net_packet);
        }
    }

    network_StopThreads();

    queue_Destroy(to_network);
    queue_Destroy(from_network);

    return 0;
}
