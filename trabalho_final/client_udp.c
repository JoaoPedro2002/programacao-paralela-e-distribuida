#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "aes.h"

#define N_SERVERS 2
#define FIRST_PORT 49152

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
  unsigned port;
  uint8_t* buffer;
  uint8_t *key;
  uint8_t vector[AES_BLOCKLEN];
} ThreadAttrs;

void* t_encrypt_block(void *args) {
  ThreadAttrs attrs = *((ThreadAttrs *) args);
  size_t size = attrs.end - attrs.start;
  uint8_t buf_fragment[size];

  for (size_t i = 0; i < size; i++) {
    buf_fragment[i] = attrs.buffer[i + attrs.start];
  }

  int sockfd;
  unsigned len;
  struct sockaddr_in address;
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = inet_addr("127.0.0.1");
  address.sin_port = htons(attrs.port);

  len = sizeof(address);

  sendto(sockfd, &size, sizeof(size_t), MSG_CONFIRM, (const struct sockaddr *)&address, len);
  sendto(sockfd, &buf_fragment, sizeof(uint8_t) * size, MSG_CONFIRM, (const struct sockaddr *)&address, len);
  sendto(sockfd, &attrs.vector, sizeof(uint8_t) * AES_BLOCKLEN, MSG_CONFIRM, (const struct sockaddr *)&address, len);
  sendto(sockfd, &attrs.key, sizeof(uint8_t) * AES_keyExpSize, MSG_CONFIRM, (const struct sockaddr *)&address, len);

  recvfrom(sockfd, &buf_fragment, sizeof(uint8_t) * size, MSG_WAITALL, (struct sockaddr *)&address, &len);
  close(sockfd);

  for (size_t i = 0; i < size; i++) {
    attrs.buffer[i + attrs.start] = buf_fragment[i];
  }

  free(args);
  pthread_exit(NULL);
}

void udp_xcrypt(struct AES_ctx* ctx, uint8_t* buffer, size_t length) {
  pthread_t t[N_SERVERS];

  for (int i = 0; i < N_SERVERS; i++) {
    ThreadAttrs* attrs = (ThreadAttrs*)malloc(sizeof(ThreadAttrs));
    attrs->start = (length / N_SERVERS) * i;
    attrs->end = (length / N_SERVERS) * (i + 1);
    memcpy(attrs->vector, ctx->Iv, AES_BLOCKLEN);
    attrs->key = ctx->RoundKey;
    attrs->buffer = buffer;
    attrs->port = i + FIRST_PORT;
    pthread_create(&t[i], NULL, t_encrypt_block, attrs);
    increment_buffer(ctx->Iv, length / (N_SERVERS * AES_BLOCKLEN));
  }

  for (int i = 0; i < N_SERVERS; i++) {
    pthread_join(t[i], NULL);
  }
}

int main(void) {
  srand((unsigned)time(NULL));
  printf("----SOCKET UDP----\n");

  size_t size;
  size_t len = 64;
  unsigned padding = 0;
  if (len % (AES_BLOCKLEN * N_SERVERS) != 0) {
    padding = (AES_BLOCKLEN * N_SERVERS) - (len % (AES_BLOCKLEN * N_SERVERS));
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
  udp_xcrypt(&ctx, buf, size);

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
  udp_xcrypt(&ctx, buf, size);

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