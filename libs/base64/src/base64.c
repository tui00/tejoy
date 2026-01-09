#include "base64.h"
#include <sodium.h>
#include <stdlib.h>
#include <string.h>

int base64_Encode(const uint8_t *data, int data_len, char **base64)
{
    if (!data || !base64)
        return -1;
    size_t b64_len = sodium_base64_encoded_len(data_len, sodium_base64_VARIANT_ORIGINAL);
    *base64 = malloc(b64_len);
    if (!*base64)
        return -1;
    sodium_bin2base64(*base64, b64_len, data, data_len, sodium_base64_VARIANT_ORIGINAL);
    return 0;
}

int base64_Decode(const char *base64, uint8_t **data)
{
    if (!base64 || !data)
        return -1;
    size_t b64_len = strlen(base64);
    size_t max_bin_len = (b64_len * 3 / 4);
    *data = malloc(max_bin_len);
    if (!*data)
        return -1;
    size_t bin_len;
    const char *end = NULL;
    if (sodium_base642bin(*data, max_bin_len, base64, b64_len, NULL, &bin_len, &end, sodium_base64_VARIANT_ORIGINAL) != 0)
    {
        free(*data);
        *data = NULL;
        return -1;
    }
    return bin_len;
}
