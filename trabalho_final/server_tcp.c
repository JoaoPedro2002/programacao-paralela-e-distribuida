#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "aes.h"

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

int main(int argc, char **argv) {
	unsigned int id;

	if(argc != 2){
		printf("Favor informar o id do socket:\n");
		printf("./server 1\n");
		return 0;
	}

	sscanf(argv[1], "%u", &id);

	int server_sockfd, client_sockfd;
	unsigned int server_len, client_len;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;

  size_t fragment_size;

	struct AES_ctx ctx;
  size_t counter;
  
  server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(49152 + id);

	server_len = sizeof(server_address);
	bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
	listen(server_sockfd, 5);

  uint8_t buffer[AES_BLOCKLEN];
  
	while(1) {		
		printf("server %d waiting\n", id);
		client_len = sizeof(client_address);
		client_sockfd = accept(server_sockfd,(struct sockaddr *)&client_address, &client_len);
		read(client_sockfd, &fragment_size, sizeof(unsigned long));
		uint8_t fragment[fragment_size];
    read(client_sockfd, &fragment, fragment_size);
    read(client_sockfd, &ctx, sizeof(struct AES_ctx));
    read(client_sockfd, &counter, sizeof(unsigned long));
    
    memcpy(&buffer, ctx.Iv, AES_BLOCKLEN);
    increment_buffer(buffer, AES_BLOCKLEN);
    
    for (size_t i = 0; i < fragment_size; i+=16) {
      Cipher((state_t*)buffer, ctx.RoundKey);
      for (int j=0; j < AES_BLOCKLEN; j++) {
        fragment[j+i] = (fragment[j+1] ^buffer[j]);
      }
    }

    phex(fragment, fragment_size);

		write(client_sockfd, &fragment, fragment_size);
		close(client_sockfd);
	}
}
