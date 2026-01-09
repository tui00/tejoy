// network.c
#include "modules/network.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/select.h>

network_Bool network_receiving = 0;
volatile network_Bool network_Stop = 1;
static pthread_t network_SenderThread;
static pthread_t network_ReceiverThread;
static struct timeval network_Timeout;

void network_DestroyPacket(void *data)
{
    network_Packet *packet = (network_Packet *)data;
    if (!packet)
        return;
    cJSON_Delete(packet->json);
    free(data);
}

static int network_SendTcpPacket(network_Packet *packet)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return -1;
    }

    struct sockaddr_in serv_addr = {.sin_family = AF_INET, .sin_port = htons(packet->port)};
    if (inet_pton(AF_INET, packet->ip, &serv_addr.sin_addr) != 1)
    {
        perror("inet_pton");
        return -1;
    }

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);

    int ret = select(sockfd + 1, &readfds, NULL, NULL, &network_Timeout);
    if (ret <= 0)
    {
        perror("select");
        close(sockfd);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0)
    {
        close(sockfd);
        return -1;
    }

    char *text = cJSON_PrintUnformatted(packet->json);
    if (!text)
    {
        close(sockfd);
        return -1;
    }

    size_t sent = send(sockfd, text, strlen(text), 0);
    close(sockfd);

    if (sent != strlen(text))
    {
        free(text);
        return -1;
    }

    free(text);
    return 0;
}

static void *network_SenderThreadFunc(void *arg)
{
    (void)arg;

    while (!network_Stop)
    {
        network_Packet *packet = queue_Pop(to_network);
        if (!packet)
        {
            sleep(1);
            continue;
        }

        int success = 0;
        for (int i = 0; i < 5; i++)
        {
            if (network_SendTcpPacket(packet) == 0)
            {
                success = 1;
                break;
            }
        }
        if (!success)
        {
            fprintf(stderr, "Failed to send packet\n");
            queue_Push(to_network, packet); // Поробуем еще раз, но сначала др. пакеты
            continue;
        }

        network_DestroyPacket(packet);
    }

    return NULL;
}

static int network_ReceiveTcpPackets(uint16_t port)
{
    int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd < 0)
    {
        perror("socket (server)");
        return -1;
    }

    struct sockaddr_in serv_addr = {.sin_family = AF_INET, .sin_addr.s_addr = INADDR_ANY, .sin_port = htons(port)};
    if (bind(server_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("bind");
        close(server_sockfd);
        return -1;
    }

    // Начинаем прослушивание
    if (listen(server_sockfd, 5) < 0)
    {
        perror("listen");
        close(server_sockfd);
        return -1;
    }

    printf("Listening on port %u...\n", port);

    network_receiving = 1;
    while (!network_Stop)
    {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(server_sockfd, &readfds);

        int ret = select(server_sockfd + 1, &readfds, NULL, NULL, &network_Timeout);
        if (ret == -1)
        {
            perror("select");
            continue;
        }
        else if (ret == 0)
            continue;

        struct sockaddr_in cli_addr;
        socklen_t cli_len = sizeof(cli_addr);
        int client_sockfd = accept(server_sockfd, (struct sockaddr *)&cli_addr, &cli_len);
        if (client_sockfd < 0)
        {
            perror("accept");
            continue;
        }

        char buffer[MAX_PACKET_SIZE];
        ssize_t received = recv(client_sockfd, buffer, sizeof(buffer) - 1, 0);
        close(client_sockfd);
        if (received > 0)
        {
            network_Packet *packet = malloc(sizeof(network_Packet));
            if (!packet)
            {
                perror("malloc (network_Packet)");
                continue;
            }

            if (inet_ntop(AF_INET, &cli_addr.sin_addr, packet->ip, sizeof(packet->ip)) == NULL)
            {
                perror("inet_ntop");
                network_DestroyPacket(packet);
                continue;
            }
            packet->port = ntohs(cli_addr.sin_port);
            packet->json = cJSON_ParseWithLength(buffer, received);
            if (!packet->json)
            {
                perror("cJSON_ParseWithLength");
                network_DestroyPacket(packet);
                continue;
            }

            if (queue_Push(from_network, packet) < 0)
            {
                fprintf(stderr, "Failed to push packet to from_network queue\n");
                network_DestroyPacket(packet);
                continue;
            }
        }
    }

    network_receiving = 0;
    close(server_sockfd);
    return 0;
}

static void *network_ReceiverThreadFunc(void *arg)
{
    uint16_t port = (uintptr_t)arg;
    if (network_ReceiveTcpPackets(port) != 0)
        printf("Failed to start receiving\n");
    return NULL;
}

int network_StartThreads(uint16_t listen_port)
{
    network_Timeout.tv_sec = 1;
    network_Timeout.tv_usec = 0;
    network_Stop = 0;

    if (pthread_create(&network_ReceiverThread, NULL, network_ReceiverThreadFunc, (void *)(uintptr_t)listen_port) != 0)
    {
        perror("pthread_create (receiver)");
        network_StopThreads();
        return -1;
    }

    if (pthread_create(&network_SenderThread, NULL, network_SenderThreadFunc, NULL) != 0)
    {
        perror("pthread_create (sender)");
        network_StopThreads();
        return -1;
    }

    return 0;
}

void network_StopThreads()
{
    network_Stop = 1;
    pthread_join(network_SenderThread, NULL);
    pthread_join(network_ReceiverThread, NULL);
}
