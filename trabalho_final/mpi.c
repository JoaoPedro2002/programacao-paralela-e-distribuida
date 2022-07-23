#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include "aes.h"

typedef struct MPI_attrs {
uint8_t* key;
uint8_t vector[AES_BLOCKLEN];
size_t start_position;
size_t size_slice;
uint8_t* buffer;
} MPI_attrs;

void create_buffer(uint8_t *buffer, unsigned long tam, unsigned padding){
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

void mpi_xcrypt(MPI_attrs *attrs) {
  for (size_t i = 0; i < attrs->size_slice; i+=AES_BLOCKLEN) {
    AES_cipher((state_t *)attrs->vector, attrs->key);
    for(int j = 0; j < AES_BLOCKLEN; j++) {
      attrs->buffer[i + j] = attrs->buffer[i + j] ^ attrs->vector[j];
    }
    increment_buffer(attrs->vector, 1);
  }
}

int main(void) {
  srand((unsigned)time(NULL));

  uint8_t *buf;
  size_t size;
  size_t len = 64;
  unsigned padding = 0;

  struct timeval t1, t2;
	double time;


  if (len % (AES_BLOCKLEN) != 0) {
    padding = (AES_BLOCKLEN) - (len % (AES_BLOCKLEN));
  }
  size = len + padding;
    
  buf = malloc(sizeof(uint8_t) * size);
  create_buffer(buf, len, padding);

  uint8_t key[16] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
  uint8_t iv[16]  = { 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff };
  struct AES_ctx ctx;
  
  AES_init_ctx_iv(&ctx, key, iv);

 	MPI_Init(NULL, NULL);

  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  #if defined(VERBOSE) && (VERBOSE==1)
  printf("%d\n",world_rank);
  #endif
  size_t slice = size / world_size;

  if (world_rank == 0) {
    printf("----MPI----\n");

    #if defined(VERBOSE) && (VERBOSE == 1)
    printf("plain:     ");
    phex(buf, size);
    printf("\n");
    #endif
    gettimeofday(&t1, NULL);
  }

  uint8_t *buffer_slice = (uint8_t *)malloc(sizeof(uint8_t) * size / world_size);

  MPI_Scatter(buf, slice, MPI_UINT8_T, buffer_slice, 
  slice, MPI_UINT8_T, 0, MPI_COMM_WORLD);
  MPI_attrs attrs;
  attrs.key = ctx.RoundKey;
  memcpy(attrs.vector, ctx.Iv, AES_BLOCKLEN);
  increment_buffer(attrs.vector, (slice / AES_BLOCKLEN) * world_rank);
  attrs.size_slice = slice;
  attrs.buffer = buffer_slice;
  mpi_xcrypt(&attrs);

  MPI_Gather(buffer_slice, slice, MPI_UINT8_T, 
  buf, slice, MPI_UINT8_T, 0, MPI_COMM_WORLD);

  free(buffer_slice);
  MPI_Barrier(MPI_COMM_WORLD);
  if (world_rank == 0) {
    #if defined(VERBOSE) && (VERBOSE == 1)
    printf("encrypted: ");
    phex(buf, size);
    printf("\n");
    #endif 
    gettimeofday(&t2, NULL);
    time = (t2.tv_sec - t1.tv_sec) + ((t2.tv_usec - t1.tv_usec)/1000000.0);
	  printf("tempo para encriptar = %f\n", time);
  
  }

  if (world_rank == 0) {
    free(buf);
  }
  MPI_Finalize();

}
