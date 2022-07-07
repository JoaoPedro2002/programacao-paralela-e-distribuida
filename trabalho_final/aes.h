#ifndef _AES_H_
#define _AES_H_

#include <stdint.h>
#include <stddef.h>

#define CTR 1
#define AES128 1

#define BLOCK_LEN 16
#define KEY_LEN 16
#define KEY_EXPECTED_SIZE 176

struct aes_context
{
  uint8_t round_key[KEY_EXPECTED_SIZE];
  uint8_t init_vector[BLOCK_LEN];
};

void aes_init(struct aes_context* context, const uint8_t* key, const uint8_t* init_vector);
void set_init_vector(struct aes_context* context, const uint8_t* init_vector);

// mesma funcao para encriptar e decriptar
void aes_ctr_xcrypt(struct aes_context* context, uint8_t* buf, size_t len);

#endif