#include <string.h>
#include <pthread.h>
#include <sys/time.h>
# include <stdlib.h>
#include "aes.h"

#define N_THREADS 2

void create_buffer(uint8_t *buffer, unsigned long tam, unsigned padding){
  for (unsigned long i = 0; i < tam; i++) {
    buffer[i] = rand() % 256;    
  }

  for (unsigned i = tam; i < (tam+ padding); i++) {
    buffer[i] = '\x04';
  }
}

void increment_buffer(uint8_t * buffer, size_t incrementions) {
  for(size_t j = 0; j < incrementions; j++) {
    for (int i = (AES_BLOCKLEN - 1); i >= 0; i--) 
    {
      if (buffer[i] == 255)
      {
        buffer[i] = 0;
        continue;
      } 
      buffer[i] += 1;
      break;   
    }
  }
}

typedef struct ThreadAttrs {
  size_t start;
  size_t end;
  uint8_t* buffer;
  uint8_t key[AES_keyExpSize];
  uint8_t vector[AES_BLOCKLEN];
} ThreadAttrs;

void* t_encrypt_block(void *args) {
  ThreadAttrs attrs = *((ThreadAttrs *) args);
  for (size_t i = attrs.start; i < attrs.end; i+=AES_BLOCKLEN) {
    AES_cipher((state_t*)attrs.vector, attrs.key);
    for (int j=0; j < AES_BLOCKLEN; j++) {
      attrs.buffer[j+i] = (attrs.buffer[j+i] ^ attrs.vector[j]);
    }
    increment_buffer(attrs.vector, 1);
  }
  free(args);
  pthread_exit(NULL);
}

void parallel_xcrypt(struct AES_ctx* ctx, uint8_t* buffer, size_t length) {
  pthread_t t[N_THREADS];
  int i;
  int ids[N_THREADS];
  /*
  Cada thread receberá um pedaco do buffer principal e um buffer que e 
  equivalente ao vetor de inicialização do AES. Esse buffer será incrementado 
  sempre que um bloco é encriptado. 
  O contador começa em um número diferente dependeando da thread.
  Exemplo valor do contador para cada thread:
         __________________
               THREAD
         __________________
    |   | t1 | t2 | t3 | t4 ...
  B | 0 | 0    7   13    19
  L | 1 | 1    8   14    .
  O | 2 | 3    9   15    .
  C | 3 | 4   10   16    .
  O | 4 | 5   11   17    .
    | 5 | 6   12   18    .
  */

  for (i = 0; i < N_THREADS; i++) {
    ids[i] = i;
    ThreadAttrs* attrs = (ThreadAttrs*)malloc(sizeof(ThreadAttrs));
    attrs->start = (length / N_THREADS) * i;
    attrs->end = (length / N_THREADS) * (i + 1);
    memcpy(attrs->vector, ctx->Iv, AES_BLOCKLEN);
    memcpy(attrs->key, ctx->RoundKey, AES_keyExpSize);
    attrs->buffer = buffer;
    pthread_create(&t[ids[i]], NULL, t_encrypt_block, attrs);
    increment_buffer(ctx->Iv, length / (N_THREADS * AES_BLOCKLEN));
  }
  
  for (i = 0; i < N_THREADS; i++) {
    pthread_join(t[i], NULL);
  }
}


int main(void) {
  srand((unsigned)time(NULL));
  printf("----PARALLEL----\n");

  size_t size;
  size_t len = 64;
  unsigned padding = 0;
  if (len % (AES_BLOCKLEN * N_THREADS) != 0) {
    padding = (AES_BLOCKLEN * N_THREADS) - (len % (AES_BLOCKLEN * N_THREADS));
  }
  size = len + padding;


  struct timeval t1, t2;
	double time;
  
  uint8_t *buf;
  buf = malloc(sizeof(uint8_t) * size);
  create_buffer(buf, len, padding);
 
  #if defined(VERBOSE) && (VERBOSE == 1)
  printf("plain: ");
  phex(buf, size);
  printf("\n");
  #endif

  uint8_t key[16] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
  uint8_t iv[16]  = { 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff };
  struct AES_ctx ctx;

  gettimeofday(&t1, NULL);
  
  AES_init_ctx_iv(&ctx, key, iv);
  parallel_xcrypt(&ctx, buf, size);

 	gettimeofday(&t2, NULL);
  
  time = (t2.tv_sec - t1.tv_sec) + ((t2.tv_usec - t1.tv_usec)/1000000.0);
	printf("tempo para encriptar = %f\n", time);

  #if defined(VERBOSE) && (VERBOSE == 1)
  printf("encrypted: ");
  phex(buf, size);
  printf("\n");
  #endif

  gettimeofday(&t1, NULL);

  AES_init_ctx_iv(&ctx, key, iv);
  parallel_xcrypt(&ctx, buf, size);

  gettimeofday(&t2, NULL);
  
  time = (t2.tv_sec - t1.tv_sec) + ((t2.tv_usec - t1.tv_usec)/1000000.0);
	printf("tempo para decriptar = %f\n", time);

  #if defined(VERBOSE) && (VERBOSE == 1)
  printf("decrypted: ");
  phex(buf, size);
  printf("\n");
  #endif

  // free(buf);
  exit(0);
}