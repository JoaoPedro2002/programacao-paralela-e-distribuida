#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include "aes.h"

#define N_THREADS 8

unsigned long tam;
uint8_t *buffer;

typedef struct ThreadAttrs {
  uint8_t i;
  int bi;
  AES_ctx ctx;
}

void* encrypt_block(void *args) {
  buf[i] = (buf[i] ^ buffer[bi]);
  pthread_exit(NULL);
}


int main(void) {
  pthread_t t[N_THREADS];
  tam = 1000;
  uint8_t buffer[AES_BLOCKLEN];
  size_t i;
  int bi;

  for (i = 0, bi = AES_BLOCKLEN; i < tam; i++, bi++) 
  {
    if (bi == AES_BLOCKLEN) {
      int thread_id = i % N_THREADS;
      pthread_create(&t[thread_id], NULL, encrypt_block, NULL);
    } else {
      buf[i] = (buf[i] ^ buffer[bi]);
    }
  }

}