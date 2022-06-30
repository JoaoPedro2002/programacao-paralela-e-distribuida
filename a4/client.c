#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define N_SERVERS 8

char *texto;
char *chute;

typedef struct Range {
  int comeco;
  int fim;
} Range;

int cria_palavra_secreta(char *palavra, int tam, int padding) {
  for (int i = 0; i < tam-1; i++)
    // sorteia algum caracter visivel, valores ASCII entre 32 e 126
    palavra[i] = 32 + rand() % 94;

	palavra[tam-1] = '\0';  

	// aplica padding de modo que TAM_PALAVRA mod N_SERVERS = 0 
	// Garante um pacote de tamanho constante para os sockets
	for (int i = tam; i < tam + padding; i++) {
		palavra[i] = '\x04'; // EOT Fim da trasmissão.
	}
}

void* t_decifra_palavra(void *args) {
	Range range = *((Range *) args);
	int size = range.fim - range.comeco;
	char text_fragment[size];

	for (int i; i < size; i++) {
		text_fragment[i] = texto[i + range.comeco];
	}

	int sockfd;
	int len;
	struct sockaddr_un address;
	int result;
	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	address.sun_family = AF_UNIX;
	strcpy(address.sun_path, "server_socket");
	len = sizeof(address); 
	result = connect(sockfd, (struct sockaddr *)&address, len);
	if(result == -1) {
		perror("oops: client");
	 	exit(1);
	}
	write(sockfd, &size, 1);
	write(sockfd, &text_fragment, 1);
	char text_response[size];
	read(sockfd, &text_fragment, 1);
	printf("response from server = %c\n", text_response);
	close(sockfd);

	for (int i; i < size; i++) {
		chute[i + range.comeco] = text_response[i];
	}
}

int main(int argc, char **argv) {
	srand((unsigned)time(NULL));

	unsigned long tam;
	pthread_t t[N_SERVERS];

	if(argc != 2){
		printf("Favor informar o tamanho da palavra. Por exemplo:\n");
		printf("./reproduz texto 100\n");
		return 0;
	}

  sscanf(argv[1],"%lu",&tam);
	int padding = N_SERVERS - (tam % N_SERVERS);

	texto = malloc(sizeof(char)*(tam + padding)); 
  chute = malloc(sizeof(char)*(tam + padding));

	cria_palavra_secreta(texto, tam, padding);

	for (int i; i < N_SERVERS; i ++) {
		Range* range = (Range*)malloc(sizeof(Range));
		range->comeco = (tam / N_SERVERS) * i;
		range->fim = (tam / N_SERVERS) * (i + 1);
		pthread_create(&t[i], NULL, t_decifra_palavra, range);
	}

	for (int i=0; i < N_SERVERS; i++) {
		pthread_join(t[i], NULL);
	}


	// int sockfd;
	// int len;
	// struct sockaddr_un address;
	// int result;
	// char ch = 'A';

	// sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	// address.sun_family = AF_UNIX;
	// strcpy(address.sun_path, "server_socket");
	// len = sizeof(address); 
	// result = connect(sockfd, (struct sockaddr *)&address, len);
	// if(result == -1) {
	// 	perror("oops: client1");
	// 	exit(1);
	// }
	// write(sockfd, &ch, 1);
	// read(sockfd, &ch, 1);
	// printf("char from server = %c\n", ch);
	// close(sockfd);
	
	exit(0);
}
