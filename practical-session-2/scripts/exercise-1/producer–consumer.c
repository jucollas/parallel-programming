#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h> 

#define BUFFER_SIZE 5
#define PRODUCE_COUNT 5

int buffer[BUFFER_SIZE];
int in = 0, out = 0;

sem_t empty;  
sem_t full;    
pthread_mutex_t mutex;

void *producer(void *arg) {
	for (int i = 0; i < PRODUCE_COUNT; i++) {
		int item = i + 1;

		sem_wait(&empty);              
		pthread_mutex_lock(&mutex);

		buffer[in] = item;
		printf("🟢 Productor produjo: %d\n", item);
		in = (in + 1) % BUFFER_SIZE;

		pthread_mutex_unlock(&mutex); 
		sem_post(&full);

		sleep(1);
	}
	pthread_exit(NULL);
}

void *consumer(void *arg) {
	for (int i = 0; i < PRODUCE_COUNT; i++) {
		sem_wait(&full);               
		pthread_mutex_lock(&mutex);   

		int item = buffer[out];
		printf("🔵 Consumidor consumió: %d\n", item);
		out = (out + 1) % BUFFER_SIZE;

		pthread_mutex_unlock(&mutex);  
		sem_post(&empty);             

		sleep(2); 
	}
	pthread_exit(NULL);
}

int main() {
	pthread_t prod, cons;

	sem_init(&empty, 0, BUFFER_SIZE);
	sem_init(&full, 0, 0);
	pthread_mutex_init(&mutex, NULL);

	pthread_create(&prod, NULL, producer, NULL);
	pthread_create(&cons, NULL, consumer, NULL);

	pthread_join(prod, NULL);
	pthread_join(cons, NULL);

	sem_destroy(&empty);
	sem_destroy(&full);
	pthread_mutex_destroy(&mutex);
	return 0;
}
