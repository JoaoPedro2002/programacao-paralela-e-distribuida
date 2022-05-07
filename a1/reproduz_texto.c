// Compilar: gcc -o reproduz_texto reproduz_texto.c -lpthread

#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define NUM_THREADS 2

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
  unsigned long tam;

  pthread_t t[NUM_THREADS];


  struct timeval t1, t2;

  if(argc != 2){
    printf("Favor informar o tamanho da palavra. Por exemplo:\n");
    printf("./reproduz texto 100\n");
    return 0;
  }

  sscanf(argv[1],"%lu",&tam);
  texto = malloc(sizeof(char)*tam); 
  chute = malloc(sizeof(char)*tam);
  cria_palavra_secreta(texto,tam);

 // Procedimento que descobre o texto
  gettimeofday(&t1, NULL);

  chute[tam-1] = '\0';
  //for (i = 0; i < tam; i++)
  //  for (j = 0; j < 255; j++){
  //    chute[i]=j;
  //    if(chute[i] == texto[i]){
  //      j=0;
  //      break;
  //     }    
  //  }
  
  long temp = tam;
  for (int i=0; i < NUM_THREADS; i++) {
    Range* range = (Range*)malloc(sizeof(Range));
    range->comeco = (tam % NUM_THREADS) * i;
    if (temp >= tam / NUM_THREADS) {
      range->fim = (tam / NUM_THREADS) * (i+1);
    } else {
      range->fim = temp;
    }
    pthread_create(&t[i], NULL, t_decifra_palavra, range);
    temp -= tam / NUM_THREADS;
  }

  for (int i=0; i < NUM_THREADS; i++) {
    pthread_join(t[i], NULL);
  }



  gettimeofday(&t2, NULL);

  //printf("Palavra secreta:    %s\n\n",texto);
  //printf("Palavra descoberta: %s\n",chute);
 
  double t_total = (t2.tv_sec - t1.tv_sec) + ((t2.tv_usec - t1.tv_usec)/1000000.0);
  printf("tempo total = %f\n", t_total);
  return 0;
}
