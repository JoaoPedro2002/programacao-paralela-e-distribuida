#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "aes.h"

#define N_SERVERS 8

typedef struct ThreadAttrs {
  size_t start;
  size_t end;
  int thread_id;
  struct AES_ctx* ctx;
  uint8_t* buf;
} ThreadAttrs;

void cria_buffer(uint8_t *buffer, unsigned long tam, unsigned padding){
  srand((unsigned)time(NULL));
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

void* t_encrypt_block(void *args) {
  ThreadAttrs attrs = *((ThreadAttrs *) args);
  uint8_t buffer[AES_BLOCKLEN];
  memcpy(buffer, attrs.ctx->Iv, AES_BLOCKLEN);
  increment_buffer(buffer, attrs.start / AES_BLOCKLEN);
  for (size_t i = attrs.start; i < attrs.end; i+=16) {
    Cipher((state_t*)buffer, attrs.ctx->RoundKey);
    for (int j=0; j < AES_BLOCKLEN; j++) {
      attrs.buf[j+i] = (attrs.buf[j+i] ^ buffer[j]);
    }
    increment_buffer(buffer, 1);
  }
  free(args);
  pthread_exit(NULL);
}

void parallel_xcrypt(struct AES_ctx* ctx, uint8_t* buf, size_t length) {
  pthread_t t[N_SERVERS];
  int i;
  int ids[N_SERVERS];
  /*
  t1 | t2 | t3 | t4 ...
  0    7   13    19
  1    8   14    .
  3    9   15    .
  4   10    .    .
  5   11    .
  6   12    .
  */

  for (i = 0; i < N_SERVERS; i++) {
    ids[i] = i;
    ThreadAttrs* attrs = (ThreadAttrs*)malloc(sizeof(ThreadAttrs));
    attrs->start = (length / N_SERVERS) * i;
    attrs->end = (length / N_SERVERS) * (i + 1);
    attrs->ctx = ctx;
    attrs->buf = buf;
    pthread_create(&t[ids[i]], NULL, t_encrypt_block, attrs);
  }
  
  for (i = 0; i < N_SERVERS; i++) {
    pthread_join(t[i], NULL);
  }

  increment_buffer(ctx->Iv, length / N_SERVERS);
}


int main(void) {
  size_t size;
  unsigned long tam = 10240000;
  unsigned padding = 0;
  if (tam % AES_BLOCKLEN != 0) {
    padding = AES_BLOCKLEN - (tam % AES_BLOCKLEN);
  }
  size = tam + padding;

  if ((size / N_SERVERS) % AES_BLOCKLEN != 0) {
    printf("O tamanho dividido pelo número de threads deve ser divisível por 16");
    exit(1);
  }

  struct timeval t1, t2;
	double time;
  
  uint8_t *buf;
  buf = malloc(sizeof(uint8_t) * size);
  cria_buffer(buf, tam, padding);

  // printf("plain: ");
  // phex(buf, size);
  // printf("\n");

  uint8_t key[16] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
  uint8_t iv[16]  = { 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff };
  struct AES_ctx ctx;

  gettimeofday(&t1, NULL);
  
  AES_init_ctx_iv(&ctx, key, iv);
  parallel_xcrypt(&ctx, buf, size);

 	gettimeofday(&t2, NULL);
  
  time = (t2.tv_sec - t1.tv_sec) + ((t2.tv_usec - t1.tv_usec)/1000000.0);
	printf("tempo = %f\n", time);


  // printf("encrypted: ");
  // phex(buf, size);
  // printf("\n");

  gettimeofday(&t1, NULL);

  AES_init_ctx_iv(&ctx, key, iv);
  parallel_xcrypt(&ctx, buf, size);

  gettimeofday(&t2, NULL);
  
  time = (t2.tv_sec - t1.tv_sec) + ((t2.tv_usec - t1.tv_usec)/1000000.0);
	printf("tempo = %f\n", time);

  // printf("decrypted: ");
  // phex(buf, size);
  // printf("\n");

  free(buf);
  exit(0);

}