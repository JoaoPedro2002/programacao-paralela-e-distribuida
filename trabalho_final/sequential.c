#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include "aes.h"

void create_buffer(uint8_t *buffer, unsigned long tam, unsigned padding){
  for (unsigned long i = 0; i < tam; i++) {
    buffer[i] = rand() % 256;    
  }

  for (unsigned i = tam; i < (tam+ padding); i++) {
    buffer[i] = '\x04';
  }
}

int main(void)
{
  srand((unsigned)time(NULL));

  struct timeval t1, t2;
  double time;

  uint8_t *buffer;
  size_t tam, size;
  tam = 1024;
  int padding = 0;
  if (tam % AES_BLOCKLEN != 0) {
    padding = AES_BLOCKLEN - (tam % AES_BLOCKLEN);
  }
  size = tam + padding;
  buffer = malloc(sizeof(uint8_t) * size);
  create_buffer(buffer, tam, padding);

  
  printf("\n-----Sequential-----\n\n");

  uint8_t key[16] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
  uint8_t iv[16]  = { 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff };
  
  struct AES_ctx ctx;

  #if defined(VERBOSE) && (VERBOSE == 1)
  printf("plain: ");
  phex(buffer, size);
  printf("\n");
  #endif

  gettimeofday(&t1, NULL);
  
  AES_init_ctx_iv(&ctx, key, iv);
  AES_CTR_xcrypt_buffer(&ctx, buffer, 64);


  gettimeofday(&t2, NULL);
  time = (t2.tv_sec - t1.tv_sec) + ((t2.tv_usec - t1.tv_usec)/1000000.0);
  printf("time encrypt = %f\n", time);

  #if defined(VERBOSE) && (VERBOSE == 1)
  printf("encrypted: ");
  phex(buffer, size);
  printf("\n");
  #endif

  gettimeofday(&t1, NULL);
  
  AES_init_ctx_iv(&ctx, key, iv);
  AES_CTR_xcrypt_buffer(&ctx, buffer, 64);

  gettimeofday(&t2, NULL);
  time = (t2.tv_sec - t1.tv_sec) + ((t2.tv_usec - t1.tv_usec)/1000000.0);
  printf("time decrypt = %f\n", time);

  #if defined(VERBOSE) && (VERBOSE == 1)
  printf("decrypted: ");
  phex(buffer, size);
  printf("\n");
  #endif

  return 0;
}
