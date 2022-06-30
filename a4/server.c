#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>

char *response;

int main() {
	int server_sockfd, client_sockfd;
	int server_len, client_len;
	struct sockaddr_un server_address;
	struct sockaddr_un client_address;
  long char_size;

	server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	server_address.sun_family = AF_UNIX;
	strcpy(server_address.sun_path, "server_socket");
	server_len = sizeof(server_address);
	bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
	listen(server_sockfd, 5);
  
	while(1) {		
		printf("server waiting\n");
		client_len = sizeof(client_address);
		client_sockfd = accept(server_sockfd,(struct sockaddr *)&client_address, &client_len);
		read(client_sockfd, &char_size, 1);
    char fragment[char_size];
    read(client_sockfd, &fragment, 1);
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
		write(client_sockfd, &fragment, 1);
		close(client_sockfd);
	}
}
