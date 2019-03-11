#ifndef CYPHER_H_
#define CYPHER_H_

#include "mbedtls/aes.h"
#include "mbedtls/base64.h"

#ifdef __cplusplus
extern "C" {
#endif

void cipher(unsigned char* input, unsigned char* output);
void decipher(unsigned char* input, unsigned char* output);

extern void __debug(const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif