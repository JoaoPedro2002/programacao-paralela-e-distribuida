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

#define N_SERVERS 4

char *texto;
char *chute;

typedef struct Range {
  int comeco;
  int fim;
	int server_id;
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
	struct sockaddr_in address;
	int result;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr("127.0.0.1");
	address.sin_port = htons(range.server_id);

	len = sizeof(address); 
	result = connect(sockfd, (struct sockaddr *)&address, len);
	if(result == -1) {
		perror("oops: client");
		free(args);
	 	pthread_exit(NULL);
	} else {
		write(sockfd, &size, sizeof(size));
		write(sockfd, &text_fragment, size);
		char response[size];
		read(sockfd, &response, size);
		printf("response from server %d = %s\n", range.server_id, response);
		close(sockfd);

		for (int i; i < size; i++) {
			chute[i + range.comeco] = response[i];
		}
		free(args);
		pthread_exit(NULL);
	}
}

int main(int argc, char **argv) {
	srand((unsigned)time(NULL));

	unsigned long tam;
	pthread_t t[N_SERVERS];

	if(argc != 2){
		printf("Favor informar o tamanho da palavra. Por exemplo:\n");
		printf("./client 100\n");
		return 0;
	}
	
	struct timeval t1, t2;
	double time;

  sscanf(argv[1],"%lu",&tam);
	int padding; 
	if (tam % N_SERVERS == 0) {
		padding = 0;
	} else {
		padding = N_SERVERS - (tam % N_SERVERS);
	}

	texto = malloc(sizeof(char)*(tam + padding)); 
  chute = malloc(sizeof(char)*(tam + padding));

	cria_palavra_secreta(texto, tam, padding);

	gettimeofday(&t1, NULL);

	for (int i; i < N_SERVERS; i ++) {
		Range* range = (Range*)malloc(sizeof(Range));
		range->comeco = ((tam+ padding) / N_SERVERS) * i;
		range->fim = ((tam + padding) / N_SERVERS) * (i + 1);
		range->server_id = i + 49152;
		pthread_create(&t[i], NULL, t_decifra_palavra, range);
	}

	for (int i=0; i < N_SERVERS; i++) {
		pthread_join(t[i], NULL);
	}

	gettimeofday(&t2, NULL);
	time = (t2.tv_sec - t1.tv_sec) + ((t2.tv_usec - t1.tv_usec)/1000000.0);
	printf("tempo = %f\n", time);

	// printf("Palavra secreta:    %s\n\n",texto);
	// printf("Palavra descoberta: %s\n",chute);

	free(texto);
	free(chute);

	exit(0);
}
