#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUMBER_OF_THREADS 100

void* read_and_increment(void*);

int shared_counter = 0;

int main() {

     pthread_t threads[NUMBER_OF_THREADS];
		 for (int i = 0; i < NUMBER_OF_THREADS; i++)
			 pthread_create(&threads[i], NULL, read_and_increment, (void*)NULL);

		 for (int i = 0; i < NUMBER_OF_THREADS; i++)
			 pthread_join(threads[i], NULL);

		 return shared_counter;
}

void* read_and_increment(void* _) {
	int counter_value = shared_counter;

	sleep(0);

	counter_value += 1;
	shared_counter = counter_value;

	return (void*)NULL;
}

