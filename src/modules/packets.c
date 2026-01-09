#include "modules/packets.h"
#include <stdint.h>
#include "sed.h"
#include "base64.h"
#include <cjson/cJSON.h>
#include <string.h>

cJSON *build_packet(uint16_t port, const uint8_t pub[sed_PUBLIC_KEY_LENGTH], const cJSON *json, const uint8_t secret[sed_SECRET_KEY_LENGTH], const uint8_t receiver_pub[sed_PUBLIC_KEY_LENGTH])
{
    if (!pub || !json || !secret || !receiver_pub)
        return NULL;

    // 1. Шифруем данные
    char *json_text = cJSON_PrintUnformatted(json);
    if (!json_text)
        return NULL;
    uint8_t *json_bin;
    uint8_t *nonce;
    int json_bin_len = sed_Encrypt(json_text, strlen(json_text), &json_bin, &nonce, secret, receiver_pub);
    free(json_text);
    if (json_bin_len < 0)
        return NULL;

    // 2. Кодируем в base64 json_bin и nonce
    char *json_b64;
    int ret = base64_Encode(json_bin, json_bin_len, &json_b64);
    free(json_bin);
    if (ret < 0)
    {
        free(nonce);
        return NULL;
    }

    char *nonce_b64;
    ret = base64_Encode(nonce, sed_NONCE_LENGTH, &nonce_b64);
    free(nonce);
    if (ret < 0)
    {
        free(json_b64);
        return NULL;
    }

    // 3. Начинаем пакет
    cJSON *packet = build_packet_without_data(port, pub);
    if (!packet)
    {
        free(json_b64);
        free(nonce_b64);
        return NULL;
    }

    // 4. Добавляем данные
    cJSON_AddStringToObject(packet, "payload", json_b64);
    free(json_b64);
    cJSON_AddStringToObject(packet, "nonce", nonce_b64);
    free(nonce_b64);

    return packet;
}
cJSON *build_packet_without_data(uint16_t port, const uint8_t pub[sed_PUBLIC_KEY_LENGTH])
{
    if (!pub)
        return NULL;
    char *pub_b64;
    if (base64_Encode(pub, sed_PUBLIC_KEY_LENGTH, &pub_b64) < 0)
        return NULL;
    cJSON *packet = cJSON_CreateObject();
    if (!packet)
    {
        free(pub_b64);
        return NULL;
    }
    cJSON_AddStringToObject(packet, "pub", pub_b64);
    cJSON_AddNumberToObject(packet, "port", port);
    free(pub_b64);
    return packet;
}

int parse_packet_without_data(const cJSON *packet, uint16_t *port, uint8_t **pub)
{
    if (!packet || !port || !pub)
        return -1;

    cJSON *port_json = cJSON_GetObjectItem(packet, "port");
    if (!cJSON_IsNumber(port_json))
        return -1;
    uint16_t _port = (uint16_t)cJSON_GetNumberValue(port_json);

    cJSON *pub_b64_json = cJSON_GetObjectItem(packet, "pub");
    if (!cJSON_IsString(pub_b64_json))
        return -1;
    char *pub_b64 = cJSON_GetStringValue(pub_b64_json);
    uint8_t *_pub;
    int pub_len = base64_Decode(pub_b64, &_pub);
    if (pub_len < 0)
        return -1;
    if (pub_len != sed_PUBLIC_KEY_LENGTH)
    {
        free(_pub);
        return -1;
    }

    if (port)
        *port = _port;
    if (pub)
        *pub = _pub;
    return 0;
}
int parse_packet(const cJSON *packet, uint16_t *port, uint8_t **pub, cJSON **json, const uint8_t secret[sed_SECRET_KEY_LENGTH])
{
    if (!packet)
        return -1;

    uint16_t _port;
    uint8_t *_pub;
    if (parse_packet_without_data(packet, &_port, &_pub) < 0)
        return -1;

    cJSON *nonce_bin_b64_json = cJSON_GetObjectItem(packet, "nonce");
    if (!cJSON_IsString(nonce_bin_b64_json))
    {
        free(_pub);
        return -1;
    }
    char *nonce_bin_b64 = cJSON_GetStringValue(nonce_bin_b64_json);
    uint8_t *nonce_bin;
    int nonce_bin_len = base64_Decode(nonce_bin_b64, &nonce_bin);
    if (nonce_bin_len < 0)
    {
        free(_pub);
        return -1;
    }
    if (nonce_bin_len != sed_NONCE_LENGTH)
    {
        free(nonce_bin);
        free(_pub);
        return -1;
    }

    cJSON *payload_json_text_bin_b64_json = cJSON_GetObjectItem(packet, "payload");
    if (!cJSON_IsString(payload_json_text_bin_b64_json))
    {
        free(nonce_bin);
        free(_pub);
        return -1;
    }
    char *payload_json_text_bin_b64 = cJSON_GetStringValue(payload_json_text_bin_b64_json);
    uint8_t *payload_json_text_bin;
    int payload_json_text_bin_len = base64_Decode(payload_json_text_bin_b64, &payload_json_text_bin);
    if (payload_json_text_bin_len < 0)
    {
        free(nonce_bin);
        free(_pub);
        return -1;
    }
    char *payload_json_text;
    int payload_json_text_len = sed_Decrypt(payload_json_text_bin, payload_json_text_bin_len, &payload_json_text, nonce_bin, secret, _pub);
    free(payload_json_text_bin);
    free(nonce_bin);
    if (payload_json_text_len < 0)
    {
        free(payload_json_text);
        free(_pub);
        return -1;
    }
    cJSON *payload_json = cJSON_ParseWithLength(payload_json_text, payload_json_text_len);
    free(payload_json_text);
    if (!payload_json)
    {
        free(_pub);
        return -1;
    }

    if (port)
        *port = _port;
    if (pub)
        *pub = _pub;
    else
        free(_pub);
    if (json)
        *json = payload_json;
    else
        cJSON_Delete(payload_json);
    return 0;
}
