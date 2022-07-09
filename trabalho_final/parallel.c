#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include "aes.h"

#define N_THREADS 64

int occupied[N_THREADS];
int thread_to_use = 0;
pthread_t t[N_THREADS];
uint8_t *buf;



typedef struct ThreadAttrs {
  uint8_t i;
  int thread_id;
  struct AES_ctx* ctx;
  uint8_t buffer[AES_BLOCKLEN];
} ThreadAttrs;

// prints string as hex
static void phex(uint8_t* str, unsigned long tam)
{
  unsigned long i;
  for (i = 0; i < tam; ++i)
      printf("%.2x", str[i]);
  printf("\n");
}

void cria_buffer(uint8_t *buffer, unsigned long tam){
  srand((unsigned)time(NULL));
  for (unsigned long i = 0; i < tam; i++) {
    buffer[i] = rand() % 256;    
  }
}


void increment_iv(struct AES_ctx* ctx) {
  for (int i = (AES_BLOCKLEN - 1); i >= 0; i--) 
  {
    if (ctx->Iv[i] == 255)
    {
      ctx->Iv[i] = 0;
      continue;
    } 
    ctx->Iv[i] += 1;
    break;   
  }
}

void* t_encrypt_block(void *args) {
  ThreadAttrs attrs = *((ThreadAttrs *) args);
  Cipher((state_t*)attrs.buffer, attrs.ctx->RoundKey);
  buf[attrs.i] = (buf[attrs.i] ^ attrs.buffer[0]);
  free(args);
  pthread_exit(NULL);
}

void parallel_xcrypt(struct AES_ctx* ctx, uint8_t* buf, size_t length) {
  size_t i;
  int bi;

  for (i = 0, bi = AES_BLOCKLEN; i < length; i++, bi++) 
  {
    if (bi == AES_BLOCKLEN) {
      // if (occupied[thread_to_use] == 1) {
      //   pthread_join(t[thread_to_use], NULL);
      // }
      occupied[thread_to_use] = 1;
      ThreadAttrs* attrs = (ThreadAttrs*)malloc(sizeof(ThreadAttrs));
      attrs->thread_id = thread_to_use;
      attrs->i = i;
      attrs->ctx = ctx;
      memcpy(attrs->buffer, ctx->Iv, AES_BLOCKLEN);
      thread_to_use = (thread_to_use + 1) % N_THREADS;
      pthread_create(&t[thread_to_use], NULL, t_encrypt_block, attrs);
      increment_iv(ctx);
      bi = 0;
    } else {
      buf[i] = (buf[i] ^ ctx->Iv[bi]);
    }
  }

  // for (int i = 0; i < N_THREADS; i++) {
  //   pthread_join(t[i], NULL);
  // }
}


int main(void) {
  for (int i = 0; i < N_THREADS; i++) {
    occupied[i] = 0;
  }
  unsigned long tam = 102400;

  uint8_t *buf;
  buf = malloc(sizeof(uint8_t) * tam);
  cria_buffer(buf, tam);

  printf("plain: ");
  phex(buf, tam);
  printf("\n");

  uint8_t key[16] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
  uint8_t iv[16]  = { 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff };
  struct AES_ctx ctx;

  AES_init_ctx_iv(&ctx, key, iv);
  parallel_xcrypt(&ctx, buf, tam);

  printf("encrypted: ");
  phex(buf, tam);
  printf("\n");

  AES_init_ctx_iv(&ctx, key, iv);
  parallel_xcrypt(&ctx, buf, tam);

  printf("decrypted: ");
  phex(buf, tam);
  printf("\n");

  free(buf);
  exit(0);

}