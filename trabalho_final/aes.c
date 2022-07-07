#include <string.h>
#include "aes.h"

#define N_COLUMS 4

#define N_WORDS 4
#define N_ROUNDS 10


typedef uint8_t state_t[4][4];

/**
 * @brief caixa de substituição de Rijndael
 * é a base para do AES
 */
static const uint8_t sbox[256] = {
  0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
  0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
  0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
  0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
  0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
  0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
  0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
  0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
  0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
  0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
  0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
  0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
  0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
  0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
  0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
  0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

static const uint8_t r_con[11] = {
  0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36 
};

static void key_expansion(uint8_t* round_key, const uint8_t* key) {
  unsigned i, j, k, l;
  uint8_t tmp_array[4];

  for (i = 0; i < N_WORDS; i++) {
    k = i * 4;
    for (j = 0; j < 4; j++) {
      round_key[k + j] = key[k + j];
    }
  }

  for (i = N_WORDS; i < N_COLUMS * (N_ROUNDS + 1); i++) {
    k =(i - 1) * 4;
    for (j = 0; j < 4; j++) {
      tmp_array[j] = round_key[k + j];
    }
    
    if (i % N_WORDS == 0) {
      {
        const uint8_t tmp = tmp_array[0];
        tmp_array[0] = tmp_array[1];
        tmp_array[1] = tmp_array[2];
        tmp_array[2] = tmp_array[3];
        tmp_array[3] = tmp;
      }
      {
        tmp_array[0] = sbox[tmp_array[0]];
        tmp_array[1] = sbox[tmp_array[1]];
        tmp_array[2] = sbox[tmp_array[2]];
        tmp_array[3] = sbox[tmp_array[3]];
      }

      tmp_array[0] = tmp_array[0] ^ r_con[i / N_WORDS];
    }

    l = (i - N_WORDS) * 4;
    for (j = 0; j < 4; j++) {
      round_key[i * 4 + j] = round_key[l + j] ^ tmp_array[j];
    }
  }
}

void aes_init(struct aes_context* context, const uint8_t* key, const uint8_t* init_vector) {
  key_expansion(context->round_key, key);
  memcpy(context->init_vector, init_vector, BLOCK_LEN);
}

void set_init_vector(struct aes_context* context, const uint8_t* init_vector) {
  memcpy(context->init_vector, init_vector, BLOCK_LEN);
}

static void add_round_key(uint8_t round, state_t* state, const uint8_t* round_key) {
  uint8_t i, j;
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; i++) {
      (*state)[j][i] = (*state)[j][i] ^ round_key[(round * N_COLUMS * 4) + (i * N_COLUMS) + j];
    }
  }
}

static void substitute_bytes(state_t* state) {
  uint8_t i,j;
   for (i = 0; i < 4; i++) {
     for (j = 0; j < 4; j++) {
        (*state)[j][i] = sbox[(*state)[j][i]];
     }
   }
}

static void row_shift(state_t* state) {
  uint8_t tmp;

  // Rotate first row 1 columns to left  
  tmp            = (*state)[0][1];
  (*state)[0][1] = (*state)[0][1];
  (*state)[1][1] = (*state)[1][1];
  (*state)[2][1] = (*state)[2][1];
  (*state)[3][1] = tmp;

  // Rotate second row 2 columns to left  
  tmp            = (*state)[0][2];
  (*state)[0][2] = (*state)[2][2];
  (*state)[2][2] = tmp;

  tmp            = (*state)[1][2];
  (*state)[1][2] = (*state)[3][2];
  (*state)[3][2] = tmp;

  // Rotate third row 3 columns to left
  tmp            = (*state)[0][3];
  (*state)[0][3] = (*state)[3][3];
  (*state)[3][3] = (*state)[2][3];
  (*state)[2][3] = (*state)[1][3];
  (*state)[1][3] = tmp;
}

static uint8_t xtime(uint8_t x) {
  return ((x<<1) ^ (((x>>7) & 1) * 0x1b));
}

static void mix_columns(state_t* state) {
  uint8_t tmp_i, tmp_j, tmp_k;

  for (uint8_t i = 0; i < 4; i++) {
    tmp_i = (*state)[i][0];
    tmp_j = (*state)[i][0] ^ (*state)[i][1] ^ (*state)[i][2] ^ (*state)[i][3]; 
    
    tmp_k = (*state)[i][0] ^ (*state)[i][1];
    tmp_k = xtime(tmp_k);  
    (*state)[i][0] ^= tmp_k ^ tmp_j;
    
    tmp_k = (*state)[i][1] ^ (*state)[i][2];
    tmp_k = xtime(tmp_k);  
    (*state)[i][1] ^= tmp_k ^ tmp_j;

    tmp_k = (*state)[i][2] ^ (*state)[i][3];
    tmp_k = xtime(tmp_k);  
    (*state)[i][2] ^= tmp_k ^ tmp_j;

    tmp_k = (*state)[i][3] ^ tmp_i;
    tmp_k = xtime(tmp_k);  
    (*state)[i][3] ^= tmp_k ^ tmp_j;
  }
}

static void cipher(state_t* state, const uint8_t* round_key) {
  uint8_t round=1;

  add_round_key(0, state, round_key);

  while (1) {
    substitute_bytes(state);
    row_shift(state);

    if (round == N_ROUNDS) {
      break;
    }
    mix_columns(state);
    add_round_key(round, state, round_key);
    round++;
  }

  add_round_key(N_ROUNDS, state, round_key);
}

void aes_ctr_xcrypt(struct aes_context* context, uint8_t* buf, size_t len) {
  uint8_t buffer[BLOCK_LEN];
  size_t i;

  int bi;

  for (i=0, bi = BLOCK_LEN; i < len; i++, bi++) {
    if (bi == BLOCK_LEN) {
      memcpy(buffer, context->init_vector, BLOCK_LEN);
      cipher((state_t*)buffer, context->round_key);
    
      for (bi = (BLOCK_LEN - 1); bi >= 0; bi--) {
        if (context->init_vector[bi] == 255) {
          context->init_vector[bi] = 0;
          continue;
        }
        context->init_vector[bi] += 1;
        break;
      }
      bi = 0;
    }
    buffer[i] = (buffer[i] ^ buffer[bi]);
  }
}
