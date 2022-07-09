#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "aes.h"

unsigned long tam;
uint8_t *buffer;

// prints string as hex
static void phex(uint8_t* str, unsigned long tam)
{
  unsigned long i;
  for (i = 0; i < tam; ++i)
      printf("%.2x", str[i]);
  printf("\n");
}

void cria_buffer(uint8_t *palavra, unsigned long tam){
  srand((unsigned)time(NULL));
  for (unsigned long i = 0; i < tam; i++) {
    palavra[i] = rand() % 256;    
  }
}

int main(void) {
  struct timeval t1, t2;
  double time;
  tam = 1024;
  buffer = malloc(sizeof(uint8_t) * tam);
  cria_buffer(buffer, tam);
  
  printf("plain: ");
  phex(buffer, tam);
  printf("\n");

  
  uint8_t key[16] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
  uint8_t iv[16]  = { 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff };

  struct AES_ctx ctx;

  gettimeofday(&t1, NULL);

  AES_init_ctx_iv(&ctx, key, iv);
  AES_CTR_xcrypt_buffer(&ctx, buffer, tam);

  printf("encrypted: ");

  phex(buffer, tam);
  printf("\n");

  AES_init_ctx_iv(&ctx, key, iv);
  AES_CTR_xcrypt_buffer(&ctx, buffer, tam);
  printf("decrypted: ");
  phex(buffer, tam);
  printf("\n");
  
  free(buffer);

  gettimeofday(&t2, NULL);
  time = (t2.tv_sec - t1.tv_sec) + ((t2.tv_usec - t1.tv_usec)/1000000.0);
  printf("tempo = %f\n", time);


  exit(0);
} 