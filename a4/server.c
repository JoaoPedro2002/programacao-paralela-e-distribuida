#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

char *response;

int main(int argc, char **argv) {
	unsigned int id;

	if(argc != 2){
		printf("Favor informar o id do socket:\n");
		printf("./server 1\n");
		return 0;
	}

	sscanf(argv[1], "%u", &id);

	int server_sockfd, client_sockfd;
	int server_len, client_len;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
  int char_size;

	server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(49152 + id);

	server_len = sizeof(server_address);
	bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
	listen(server_sockfd, 5);
  
	while(1) {		
		printf("server %d waiting\n", id);
		client_len = sizeof(client_address);
		client_sockfd = accept(server_sockfd,(struct sockaddr *)&client_address, &client_len);
		read(client_sockfd, &char_size, sizeof(int));
		printf("%d\n", char_size);
		char fragment[char_size];
    read(client_sockfd, &fragment, char_size);
		printf("%s\n", fragment);
    response = malloc(sizeof(char) * char_size);
    for (int i=0; i < char_size; i++) {
      for (int j=0; j < 255; j++) {
        response[i] = j;
        if (response[i] == fragment[i]) {
          j=0;
          break;
        }
      }
    }
		
		printf("Response: %s\n", response);
		write(client_sockfd, &response, char_size);
		close(client_sockfd);
	}
}
