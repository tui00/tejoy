// packets.h
#ifndef PACKETS_H
#define PACKETS_H

#include <stdint.h>
#include <cjson/cJSON.h>
#include "sed.h"

cJSON *build_packet(uint16_t port, const uint8_t pub[sed_PUBLIC_KEY_LENGTH], const cJSON *json, const uint8_t secret[sed_SECRET_KEY_LENGTH], const uint8_t receiver_pub[sed_PUBLIC_KEY_LENGTH]);
cJSON *build_packet_without_data(uint16_t port, const uint8_t pub[sed_PUBLIC_KEY_LENGTH]);

int parse_packet_without_data(const cJSON *packet, uint16_t *port, uint8_t **pub);
int parse_packet(const cJSON *packet, uint16_t *port, uint8_t **pub, cJSON **json, const uint8_t secret[sed_SECRET_KEY_LENGTH]);

#endif // PACKETS_H
