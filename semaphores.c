/************************* 
   * This program uses semaphores to synchronize the printing of "What a Wonderful World!" iteratively on the terminal using threads * 
**************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

/*For multi threading*/
#include <pthread.h>

/*For using the semaphore API*/
#include <semaphore.h>

/*Global semaphores used for synchronizing the output. */
sem_t A; /* For the first part of text */
sem_t B; /* For the second part of text */
sem_t C; /* For the third part of text */

/*These are the thread function that will work as our thread routines each doing their respective tasks. */

/*The following thread function prints "What A " to the terminal at the right moment. */
void *first_function(void *data)
{
	while (1)
	{
		/*We wait for the last part of the sentence to be written before we write, 
	the first part again. */

		sem_wait(&C);
		printf("What A ");

		/*
	   We post on 'A' indicating the first part is written. 
	*/
		sem_post(&A);
	}
	return NULL;
}

/*The following thread function prints "Wonderful " to the terminal at the right moment.  */
void *second_function(void *data)
{
	while (1)
	{
		/*This thread waits for the first part of the sentence to be written*/
		sem_wait(&A);

		printf("Wonderful ");

		/*
	   We post on 'B' indicating the middle part is written. 
	*/
		sem_post(&B);
	}
	return NULL;
}

/*The following thread function prints "World! " to the terminal at the right moment.  */
void *third_function(void *data)
{
	while (1)
	{
		/*We wait for the middle part to be written.  */
		sem_wait(&B);

		printf("World! ");

		/* We post on 'C' to indicate that the last part has been written */ 
		sem_post(&C);
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	
	/*We initialize the semaphores to proper values. Note we are specifying zero for the second argument since these semaphores are only shared between threads*/

	sem_init(&A, 0, 0);
	sem_init(&B, 0, 0);
	sem_init(&C, 0, 1);

	/*Array to store the thread IDs for our three threads. */

	pthread_t tids[3];

	/*We spawn the three threads. */
	pthread_create(&tids[0], NULL, first_function, NULL);
	pthread_create(&tids[1], NULL, second_function, NULL);
	pthread_create(&tids[2], NULL, third_function, NULL);

	/*We join the threads to our main thread. */
	for (int i = 0; i < 3; i++)
	{
		pthread_join(tids[i], NULL);
	}

	/*We destroy the semaphores after use. */
	sem_destroy(&A);
	sem_destroy(&B);
	sem_destroy(&C);

	return 0;
}