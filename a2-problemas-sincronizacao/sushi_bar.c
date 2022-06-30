#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>

#define N_THREADS 20
#define N_SEATS 5

int eating = 0, waiting = 0;

sem_t mutex;
sem_t block;

bool must_wait = false;

void eat_sushi(int id) {
    printf("Customer %d is eating sushi.\n", id);
    sleep(1 + rand() % 4);
    printf("Customer %d stopped eating sushi.\n", id);
    return;
}

void *thread_customer(void* arg) {
	int id = *((int*)arg);

    printf("Customer %d arrived at the bar.\n", id);
    sleep(1 + rand() % 4);
    sem_wait(&mutex);
    if (must_wait) {
        waiting++;
        sem_post(&mutex);
        sem_wait(&block);
        waiting--;
    } 
    
    eating++;
    must_wait = (eating == N_SEATS);
    sem_post(&mutex);
    
    if (waiting > 0 && !must_wait) sem_post(&block);
    else sem_post(&mutex);

    eat_sushi(id);

    sem_wait(&mutex);
    eating--;
    if (eating == 0) must_wait = false;

    if (waiting > 0 && !must_wait) sem_post(&block);
    else sem_post(&mutex);

    pthread_exit(NULL);
}

int main(int argc, char** argv) {
    srand((unsigned)time(NULL));

   	pthread_t threads[N_THREADS];

    sem_init(&mutex, 0, 1);
	sem_init(&block, 0, 0);

    for (int i = 0; i < N_THREADS; i++) {
		pthread_create(&threads[i], NULL, thread_customer, (void *) &i);
	}

	for (int i = 0; i < N_THREADS; i++) {
		pthread_join(threads[i], NULL);	
	}

    sem_destroy(&mutex);
    sem_destroy(&block);
    
    return 0;
}