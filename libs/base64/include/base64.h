// base64.h
#ifndef BASE64_H
#define BASE64_H

#include <stdint.h>

int base64_Encode(const uint8_t *data, int data_len, char **base64);
int base64_Decode(const char *base64, uint8_t **data);

#endif
