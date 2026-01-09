// sed.c
#include "sed.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sodium.h>

int sed_Init() { return sodium_init(); }
int sed_GenerateKeys(uint8_t secret_key[sed_SECRET_KEY_LENGTH], uint8_t public_key[sed_PUBLIC_KEY_LENGTH]) { return crypto_box_keypair(public_key, secret_key); }

int sed_Encrypt(const char *text, int text_len, uint8_t **cipher, uint8_t **nonce,
                const uint8_t sk_a[sed_SECRET_KEY_LENGTH], const uint8_t pk_b[sed_PUBLIC_KEY_LENGTH])
{
    // 1. Проверка аргументов
    if (!text || text_len <= 0 || !cipher || !nonce || !sk_a || !pk_b)
        return -1;

    // 2. Генерация nonce
    *nonce = malloc(sed_NONCE_LENGTH);
    if (!*nonce)
        return -2;
    randombytes_buf(*nonce, sed_NONCE_LENGTH);

    // 3. Вычисление длины шифра
    int cipher_len = text_len + crypto_box_MACBYTES;
    *cipher = malloc(cipher_len);
    if (!*cipher)
    {
        free(*nonce);
        return -2;
    }

    // 4. Шифрование
    if (crypto_box_easy(
            *cipher,               // выходной буфер
            (const uint8_t *)text, // текст
            text_len,              // длина текста
            *nonce,                // nonce
            pk_b,                  // публичный ключ B
            sk_a                   // секретный ключ A
            ) != 0)
    {
        free(*cipher);
        free(*nonce);
        return -3;
    }

    return cipher_len;
}

int sed_Decrypt(const uint8_t *cipher, int cipher_len, char **plaintext, const uint8_t *nonce,
                const uint8_t sk_b[sed_SECRET_KEY_LENGTH], const uint8_t pk_a[sed_PUBLIC_KEY_LENGTH])
{
    // 1. Проверка аргументов
    if (!cipher || cipher_len <= 0 || !plaintext || !nonce || !sk_b || !pk_a)
        return -1;

    // 2. Длина открытого текста
    int plaintext_len = cipher_len - crypto_box_MACBYTES;
    if (plaintext_len < 0)
        return -3;

    *plaintext = malloc(plaintext_len + 1);
    if (!*plaintext)
        return -2;

    // 3. Расшифрование
    if (crypto_box_open_easy(
            (uint8_t *)*plaintext, // выходной буфер
            cipher,                // шифр
            cipher_len,            // длина шифра
            nonce,                 // nonce
            pk_a,                  // публичный ключ A
            sk_b                   // секретный ключ B
            ) != 0)
    {
        free(*plaintext);
        return -3;
    }

    (*plaintext)[plaintext_len] = '\0';
    return plaintext_len;
}
