#ifndef _AES_H_
#define _AES_H_

#define VERBOSE 1

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#define AES_BLOCKLEN 16 // Block length in bytes - AES is 128b block only


#define AES_KEYLEN 16   // Key length in bytes
#define AES_keyExpSize 176

struct AES_ctx
{
  uint8_t RoundKey[AES_keyExpSize];
  uint8_t Iv[AES_BLOCKLEN];
};

// state - guarda os resultados intermediários durante o processo de encriptar ou decriptar
typedef uint8_t state_t[4][4];

void AES_init_ctx(struct AES_ctx* ctx, const uint8_t* key);
void AES_init_ctx_iv(struct AES_ctx* ctx, const uint8_t* key, const uint8_t* iv);
void AES_ctx_set_iv(struct AES_ctx* ctx, const uint8_t* iv);

void AES_cipher(state_t* state, const uint8_t* RoundKey);

// Same function for encrypting as for decrypting. 
// IV is incremented for every block, and used after encryption as XOR-compliment for output
// Suggesting https://en.wikipedia.org/wiki/Padding_(cryptography)#PKCS7 for padding scheme
// NOTES: you need to set IV in ctx with AES_init_ctx_iv() or AES_ctx_set_iv()
//        no IV should ever be reused with the same key 

void AES_CTR_xcrypt_buffer(struct AES_ctx* ctx, uint8_t* buf, size_t length);

// prints string as hex
void phex(uint8_t* str, unsigned long tam);

#endif // _AES_H_