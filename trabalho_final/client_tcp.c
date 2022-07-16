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

#define N_SERVERS 2
#define FIRST_PORT 49152

typedef struct ThreadAttrs {
  size_t start;
  size_t end;
  int port;
  uint8_t *buf;
  struct AES_ctx* ctx;
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
  size_t size = attrs.end - attrs.start;
  uint8_t buf_fragment[size];

  for (size_t i = 0; i < size; i++) {
    buf_fragment[i] = attrs.buf[i + attrs.end];
  }

  int sockfd;
  int len;
  struct sockaddr_in address;
  int result;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = inet_addr("127.0.0.1");
  address.sin_port = htons(attrs.port);

  len = sizeof(address);
  result = connect(sockfd, (struct sockaddr *)&address, len);
  if (result == -1) {
    perror("oops: client");
  } else {
    size_t counter = attrs.start / AES_BLOCKLEN;
    write(sockfd, &size, sizeof(size));
    write(sockfd, &buf_fragment, size);
    write(sockfd, attrs.ctx, sizeof(struct AES_ctx));
    write(sockfd, &counter, sizeof(size_t));
    uint8_t response[size];
    read(sockfd, &response, size);
    close(sockfd);

    for (size_t i = 0; i < size; i++) {
      attrs.buf[i + attrs.start] = response[i];
    }
  }

  free(args);
  pthread_exit(NULL);
}

void tcp_xcrypt(struct AES_ctx* ctx, uint8_t* buf, size_t length) {
  pthread_t t[N_SERVERS];
  int ids[N_SERVERS];

  for (int i = 0; i < N_SERVERS; i++) {
    ThreadAttrs* attrs = (ThreadAttrs*)malloc(sizeof(ThreadAttrs));
    attrs->start = (length / N_SERVERS) * i;
    attrs->end = (length / N_SERVERS) * (i + i);
    attrs->ctx = ctx;
    attrs->buf = buf;
    attrs->port = i + 49152;
    pthread_create(&t[i], NULL, t_encrypt_block, attrs);
  }

  for (int i = 0; i < N_SERVERS; i++) {
    pthread_join(t[i], NULL);
  }
  increment_buffer(ctx->Iv, length / N_SERVERS);
}

int main(void) {
  printf("----SOCKET TCP----");

  size_t size;
  size_t len = 256;
  unsigned padding = 0;
  if (len % AES_BLOCKLEN != 0) {
    padding = AES_BLOCKLEN - (len % AES_BLOCKLEN);
  }
  size = len + padding;

  if ((size / N_SERVERS) % AES_BLOCKLEN != 0) {
    printf("O tamanho dividido pelo número de threads deve ser divisível por 16");
    exit(1);
  }

  struct timeval t1, t2;
	double time;
  
  uint8_t *buf;
  buf = malloc(sizeof(uint8_t) * size);
  cria_buffer(buf, len, padding);

  uint8_t key[16] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
  uint8_t iv[16]  = { 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff };
  struct AES_ctx ctx;

  gettimeofday(&t1, NULL);
  
  AES_init_ctx_iv(&ctx, key, iv);
  tcp_xcrypt(&ctx, buf, size);

 	gettimeofday(&t2, NULL);

  time = (t2.tv_sec - t1.tv_sec) + ((t2.tv_usec - t1.tv_usec)/1000000.0);
	printf("tempo para encriptar = %f\n", time);


  printf("encrypted: ");
  phex(buf, size);
  printf("\n");

  gettimeofday(&t1, NULL);

  AES_init_ctx_iv(&ctx, key, iv);
  tcp_xcrypt(&ctx, buf, size);

  gettimeofday(&t2, NULL);
  
  time = (t2.tv_sec - t1.tv_sec) + ((t2.tv_usec - t1.tv_usec)/1000000.0);
	printf("tempo para decriptar = %f\n", time);

  printf("decrypted: ");
  phex(buf, size);
  printf("\n");

  free(buf);
  exit(0);
}