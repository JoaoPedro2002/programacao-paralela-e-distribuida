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

/**
 * @brief inicializa o contexto AES
 * 
 * @param context uma struct contendo o contexto 
 * @param key a chave
 * @param init_vector o vetor de inicialização 
 */
void aes_init(struct aes_context* context, const uint8_t* key, const uint8_t* init_vector);

/**
 * @brief atribui um valor para o vetor de inicialização
 * 
 * @param context contexto que terá seu valor alterado
 * @param init_vector vetor de inicalização
 */
void set_init_vector(struct aes_context* context, const uint8_t* init_vector);

/**
 * @brief função utilizada para encriptar e decriptar um buffer.
 * 
 * A implementação CTR, não demanda uma separação das funções para encriptar 
 * e decriptar dados
 * @param context contexto AES
 * @param buf buffer
 * @param len tamanho
 */
void aes_ctr_xcrypt(struct aes_context* context, uint8_t* buf, size_t len);

#endif