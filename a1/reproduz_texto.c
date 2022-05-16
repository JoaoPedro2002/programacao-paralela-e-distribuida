// Compilar: gcc -o reproduz_texto reproduz_texto.c -lpthread

#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <math.h>

#define ITERATIONS 100


char *texto;
char *chute;

typedef struct Range {
  int comeco;
  int fim;
} Range;

int cria_palavra_secreta(char *palavra, int tam){
  srand((unsigned)time(NULL));
  for (int i = 0; i < tam-1; i++)
    // sorteia algum caracter visivel, valores ASCII entre 32 e 126
    palavra[i] = 32 + rand() % 94;    
  palavra[tam-1] = '\0';  
}

void* t_decifra_palavra(void *args) {
  Range range = *((Range *) args);
  
  //printf("%d\n", range.comeco);
  //printf("%d\n", range.fim);
  for (long i=range.comeco; i < range.fim; i++) {
    for (short j = 0; j < 255; j++){
      chute[i]=j;
      if(chute[i] == texto[i]){
        j=0;
        break;
      }   
    }
  }

  free(args);
  pthread_exit(NULL); 
}

int main(int argc, char **argv){
  double average_time = 0;
  unsigned long tam;
  unsigned int n_threads;

  struct timeval t1, t2;

  if(argc != 3){
    printf("Favor informar o tamanho da palavra e no numero de threads. Por exemplo:\n");
    printf("./reproduz texto 100 2 \n");
    return 0;
  }

  sscanf(argv[1],"%lu",&tam);
  sscanf(argv[2], "%u", &n_threads);

  printf("Executando 100 vezes com palavra de tamanho %ld e com %d threads\n", tam, n_threads);

  pthread_t t[n_threads];
  
  texto = malloc(sizeof(char)*tam); 
  chute = malloc(sizeof(char)*tam);

  for (int j = 0; j < ITERATIONS; j++) {
    cria_palavra_secreta(texto,tam);

    gettimeofday(&t1, NULL);

    chute[tam-1] = '\0';
    for (int i=0; i < n_threads; i++) {
      Range* range = (Range*)malloc(sizeof(Range));
      range->comeco = floor(tam / n_threads) * i;
      if (i == n_threads - 1) {
        range->fim = tam;
      } else {
        range->fim = floor(tam / n_threads) * (i + 1);
      }    
      pthread_create(&t[i], NULL, t_decifra_palavra, range);
    }

    for (int i=0; i < n_threads; i++) {
      pthread_join(t[i], NULL);
    }

    gettimeofday(&t2, NULL);

    //printf("Palavra secreta:    %s\n\n",texto);
    //printf("Palavra descoberta: %s\n",chute);

    // Adiciona o tempo de execução  
    average_time += (t2.tv_sec - t1.tv_sec) + ((t2.tv_usec - t1.tv_usec)/1000000.0);
  }
  average_time = average_time / ITERATIONS;
  printf("tempo medio = %f\n", average_time);
  return 0;
}
