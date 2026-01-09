// sed.h
#ifndef SED_H
#define SED_H

#include <stdint.h>

#include <sodium.h>

#define sed_PUBLIC_KEY_LENGTH crypto_sign_PUBLICKEYBYTES
#define sed_SECRET_KEY_LENGTH crypto_sign_SECRETKEYBYTES
#define sed_NONCE_LENGTH crypto_box_NONCEBYTES

#define sed_GENERATE_KEYS(secret, public)                                  \
    uint8_t secret[sed_SECRET_KEY_LENGTH], public[sed_PUBLIC_KEY_LENGTH]; \
    sed_GenerateKeys(secret, public)

int sed_Init();
int sed_GenerateKeys(uint8_t secret_key[sed_SECRET_KEY_LENGTH], uint8_t public_key[sed_PUBLIC_KEY_LENGTH]);

/**
 * Шифрует текст через crypto_secretbox
 * @param text Текст для шифрования
 * @param text_len Длина текста
 * @param cipher Выделеный выходной буфер для шифра
 * @param nonce Выделеный выходной буфер для nonce
 * @param sk_a Секретный ключ стороны A
 * @param pk_b Публичный ключ стороны B
 * @return Длину шифра или -1 при неверных аргументов, -2 при ошибке памяти, -3 при ошибке расшифровки
 */
int sed_Encrypt(const char *text, int text_len, uint8_t **cipher, uint8_t **nonce,
                const uint8_t sk_a[sed_SECRET_KEY_LENGTH], const uint8_t pk_b[sed_PUBLIC_KEY_LENGTH]);

/**
 * Расшифровавоет текст через crypto_box_open
 * @param cipher Шифр для расшифрования
 * @param cipher_len Длина шифра
 * @param text Выделеный выходной буфер для текста
 * @param nonce Входной буфер с nonce
 * @param sk_b Секретный ключ стороны B
 * @param pk_a Публичный ключ стороны A
 * @return Длину текста или -1 при неверных аргументов, -2 при ошибке памяти, -3 при ошибке расшифровки
 */
int sed_Decrypt(const uint8_t *cipher, int cipher_len, char **plaintext, const uint8_t *nonce,
                const uint8_t sk_b[sed_SECRET_KEY_LENGTH], const uint8_t pk_a[sed_PUBLIC_KEY_LENGTH]);

#endif
