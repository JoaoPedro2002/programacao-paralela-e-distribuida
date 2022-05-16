// Compilar: gcc -o reproduz_texto_sequencial reproduz_texto_sequencial.c

#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <time.h>

#define ITERATIONS 100

char *texto;
char *chute;

int cria_palavra_secreta(char *palavra, int tam){
  srand((unsigned)time(NULL));
  for (int i = 0; i < tam-1; i++)
    // sorteia algum caracter visivel, valores ASCII entre 32 e 126
    palavra[i] = 32 + rand() % 94;    
  palavra[tam-1] = '\0';  
}

int main(int argc, char **argv){
  double average_time = 0;
  unsigned long i,tam;
  short j, k;

  struct timeval t1, t2;

  if(argc != 2){
    printf("Favor informar o tamanho da palavra. Por exemplo:\n");
    printf("./reproduz texto 100\n");
    return 0;
  }

  sscanf(argv[1],"%lu",&tam);
  texto = malloc(sizeof(char)*tam); 
  chute = malloc(sizeof(char)*tam);

  for (k = 0; k < ITERATIONS; k++) {
  
    cria_palavra_secreta(texto,tam);

    gettimeofday(&t1, NULL);

    // Procedimento que descobre o texto
    chute[tam-1] = '\0';
    for (i = 0; i < tam; i++)    
      for (j = 0; j < 255; j++){
      chute[i]=j;
        if(chute[i] == texto[i]){
          j=0;
          break;
        }    
      }

    gettimeofday(&t2, NULL);

    printf("Palavra secreta:    %s\n\n",texto);
    printf("Palavra descoberta: %s\n",chute);
    // Adiciona o tempo de execução  
    average_time += (t2.tv_sec - t1.tv_sec) + ((t2.tv_usec - t1.tv_usec)/1000000.0);
  }

  average_time = average_time / ITERATIONS;
  printf("tempo medio = %f\n", average_time);
 
  return 0;
}
