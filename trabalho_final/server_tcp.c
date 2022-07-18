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

#define FIRST_PORT 49152


void increment_buffer(uint8_t * buffer) {
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


int main(int argc, char **argv) {
	unsigned id, port;

	if(argc != 2){
		printf("Favor informar o id do socket:\n");
		printf("./server 1\n");
		return 0;
	}

	sscanf(argv[1], "%u", &id);
  port = FIRST_PORT + id;

	int server_sockfd, client_sockfd;
	unsigned server_len, client_len;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;

  size_t fragment_size;
  uint8_t vector[AES_BLOCKLEN];
  uint8_t key[AES_keyExpSize];
  
  server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(port);

	server_len = sizeof(server_address);
	bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
	listen(server_sockfd, 5);
  
	while(1) {		
		printf("server %u waiting on port %u\n", id, port);
		client_len = sizeof(client_address);
		client_sockfd = accept(server_sockfd,(struct sockaddr *)&client_address, &client_len);
		read(client_sockfd, &fragment_size, sizeof(size_t));
		uint8_t fragment[fragment_size];
    read(client_sockfd, &fragment, fragment_size * sizeof(uint8_t));
    read(client_sockfd, &vector, sizeof(uint8_t) * AES_BLOCKLEN);
    read(client_sockfd, &key, sizeof(uint8_t) * AES_keyExpSize);
    
    #if defined(VERBOSE) && (VERBOSE == 1)
    printf("plain:\n");
    phex(fragment, fragment_size);
    printf("\n");
    #endif

    for (size_t i = 0; i < fragment_size; i+=AES_BLOCKLEN) {
      AES_cipher((state_t*)vector, key);
      for (int j=0; j < AES_BLOCKLEN; j++) {
        fragment[j+i] = (fragment[j+i] ^ vector[j]);
      }
      increment_buffer(vector);
    }

    #if defined(VERBOSE) && (VERBOSE == 1)
    printf("encrypted:\n");
    phex(fragment, fragment_size);
    printf("\n");
    #endif

		write(client_sockfd, &fragment, fragment_size * sizeof(uint8_t));
		close(client_sockfd);
	}
}
