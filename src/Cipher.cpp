#include "Cipher.h"
#include <stdio.h>
#include <string.h>

const char* key = "{22A0F376-2FE1-496E-9BE5-6797DF9844FA}";

void cipher(unsigned char* input, unsigned char* output) {
  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_enc(&aes, (const unsigned char*) key, strlen(key) * 8);
  mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, (const unsigned char*)input, output);
  mbedtls_aes_free(&aes);
}

void decipher(unsigned char* input, unsigned char* output) {
  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_enc(&aes, (const unsigned char*) key, strlen(key) * 8);
  mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT, (const unsigned char*)input, output);
  mbedtls_aes_free(&aes);
}

