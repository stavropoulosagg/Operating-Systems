/****************************
  Multi threaded vector dot product application. 
  
  * We represent vectors as arrays * 
  * Dot product is only defined when the vectors have equal number of dimensions * 
  
*****************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

/* For multi threading. */
#include <pthread.h>

/*Global variable that will store the total sum of all partial sums. 
  This variable will be protected using mutex locks and it is a critical section. 
  Infact, it will store the result of the dot product operation after all threads have successfully completed their jobs. 
*/
float total_sum = 0;

/* Struct that stores important information that will be passed to a thread. */
typedef struct THREAD_DATA
{
	int thread_num; /*The number we assign to identify our thread. */

	/*Pointers to the segments from where we need to start taking the dot product. */
	float *seg_a;
	float *seg_b;

	int num; // The number of elements for which dot product should be taken in this segment of the vectors.
} THREAD_DATA;

/*** Global mutex object ***/
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/** The above mutex will be used for accessing critical sections of the code **/

/**** Declaration of important functions *****/

// Helper function that prints vectors on the terminal as a list.
void print_vector(char sym, const float *vec, int n);

// Vector inner product function.
float vec_dot_product(const float *x, const float *y, uint32_t n);

// Service thread function that will work as our thread.
void *service_thread(void *data);

/*The main entry point of our application. */
int main(int argc, char *argv[])
{
	/*User supplied option */
	int op = 0;

	/*Number of threads requested by the user. */
	int p = 0;

	/*Maximum dimension of the vector we will be working with, as specified by the user*/
	int n = 0;

	/*The two vectors, whose dot product will be calculated. */
	float *A = NULL;
	float *B = NULL;

	/*Heap allocated arrays of thread IDs*/
	pthread_t *tids;

	while (1)
	{
		/*We reset the global variables... */
		total_sum = 0.0;

		/*Display the menu before the user... */
		printf("\n\n##### VECTOR DOT PRODUCT (MULTI THREADED) #####\n");
		printf("\nPlease choose an option: \n");
		printf("\t1. Calculate Vector Dot Product. \n\t2. Benchmark performance. \n\t3. Exit application.\nOption > ");

		/*Read the option input from the user. */
		scanf("%d", &op);

		if (op == 1)
		{
			printf("\n");

			/*Read the number of threads required from the user. */
			printf("Please, enter the number of threads required: ");
			scanf("%d", &p);

			/*Read in, the maximum dimension of the vectors that we will be dealing with... */
			printf("\n\nPlease, enter the dimension of the vectors: ");
			scanf("%d", &n);

			/*Check the validity of the provided values. */

			if (n <= 0)
			{
				printf("\n[!]Sorry, dimension of vectors MUST be greater than zero... \n   Please, try again!\n");
				continue;
			}

			if (p <= 0)
			{
				printf("\n[!]Sorry, number of threads required MUST be greater than zero... \n   Please, try again!\n");
				continue;
			}

			/*Check whether the number of threads evenly divides the dimension provided. This is to give equal work load balance to all threads spawned. */
			if ((n % p) != 0)
			{
				printf("\n[!]Please, provide a dimension that is divisible by the number of threads provided and try again ! \n");
				continue;
			}

			/*We allocate memory for the storing the two vectors of  "n" dimensions we will be processing... */

			A = (float *)malloc(n * sizeof(float));
			B = (float *)malloc(n * sizeof(float));

			/*We read all the "n" components of the FIRST vector (A) from the user... */
			printf("\n[*] ### For the FIRST vector (A)... ###\n\n");
			for (int i = 0; i < n; i++)
			{
				printf("Please, provide component A[%d]: ", i);

				scanf("%f", &A[i]);

				printf("\n");
			}

			/*We read all the "n" components of the SECOND vector (B) from the user... */
			printf("\n[*] ### For the SECOND vector (B)... ###\n\n");
			for (int i = 0; i < n; i++)
			{
				printf("Please, provide component B[%d]: ", i);

				scanf("%f", &B[i]);

				printf("\n");
			}

			/*Now since, we have read all the components of both vectors A and B from the user, we can proceed to calculate the vector dot product using threads. */

			/*We allocate memory for storing the thread IDs of all "p" threads that will be spawned. */
			tids = (pthread_t *)malloc(p * sizeof(pthread_t));

			printf("\n[*]### Calculating dot product of 'A' and 'B' ###\n");

			/*We start off the individual threads using `pthread_create()` */

			/*Each thread will work on its allotted segment in the vector array. Every thread needs to process n / p elements. */
			for (int i = 0; i < p; i++)
			{
				/*We allocate memory for data that needs to be passed to the individual thread. 
			*/
				THREAD_DATA *tdata = (THREAD_DATA *)malloc(sizeof(THREAD_DATA));
				/*We assign numbers for our threads. */
				(tdata)->thread_num = (i + 1);

				/*We want this thread to work on (n / p) elements starting from the indicated position for both input vectors. 
				The thread will compute the partial sum from the offset upto n/p elements. 
				*/

				(tdata)->seg_a = A + (n / p) * i;
				(tdata)->seg_b = B + (n / p) * i;
				/* We tell the thread the size of the segment for which partial sum needs to be completed. */
				(tdata)->num = n / p;

				/*Finally we spawn the thread with the above data. */

				pthread_create(&tids[i], NULL, service_thread, (void *)(tdata));
			}

			/*After the threads are successfully dispatched to perform the dot product on their respective segments. We wait for all threads to finish, and finally join to the main thread.  */

			for (int i = 0; i < p; i++)
			{
				/* Join the thread based on their thread ID. */
				pthread_join(tids[i], NULL);
			}

			/*Now, we can successfully print the results before the user. */

			printf("\n[*]SUCCESS: Inner product of vectors A and B successfully calculated. \n   Scroll down to see results... \n\n");

			print_vector('A', A, n);

			printf("\n\n");

			print_vector('B', B, n);

			printf("\n");

			printf("\n[*]RESULT of A dot B = %f", total_sum);

			/*Deallocate all heap allocated memory*/
			free(A);
			free(B);
			free(tids);
		}
		else if (op == 2)
		{
			/* We enter into benchmarking mode, where we can see how multi threading helps in increasing the performance of dot product computation. */
			printf("\n[*]Please, provide the parameters so that performance can be benchmarked... \n\n");
			/*Read the dimension and number of threads required. */
			printf("\nPlease, provide dimension: ");

			scanf("%d", &n);

			printf("\nPlease, provide number of threads required: ");

			scanf("%d", &p);

			/*Check whether the number of threads evenly divides the dimension provided. This is to give equal work load balance to all threads spawned. */
			if ((n % p) != 0)
			{
				printf("\n[!]Please, provide a dimension that is divisible by the number of threads provided and try again ! \n");
				continue;
			}

			/*We allocate memory for the two vectors "A" and "B" both having equal dimensions (n) */
			A = (float *)malloc(n * sizeof(float));
			B = (float *)malloc(n * sizeof(float));

			/*We allocate an array of thread IDs */
			tids = (pthread_t *)malloc(p * sizeof(pthread_t));

			printf("\n[*]Filling the %d dimensional vectors 'A' and 'B' with random float data... \n", n);

			/*We seed the random number generator*/
			srandom(time(0));

			for (int i = 0; i < n; i++)
			{
				A[i] = (float)random() / (float)RAND_MAX;

				B[i] = (float)random() / (float)RAND_MAX;
			}

			printf("\nDo you want to print the randomly generated test vectors A and B? (Y / N) ");
			char ch=getc(stdin);
			scanf("%c", &ch);

			if (ch == 'y' || ch == 'Y')
			{
				/*We print the randomly generated vectors of user provided dimensions on the terminal. */
				printf("\n[*]Randomly generated %d-dimensional vector \n", n);
				print_vector('A', A, n);

				printf("\n\n[*]Randomly generated %d-dimensional vector \n", n);

				print_vector('B', B, n);
			}
			else
			{
				printf("\n[*]Randomly generated %d-dimensional vectors 'A' and 'B' \n", n); 
			}

			printf("\n[*]### Calculating dot product of 'A' and 'B' ###\n");

			/*We spawn the user requested number of threads, and also assign them the bounds of the vectors for which partial sums needs to be computed. */

			for (int i = 0; i < p; i++)
			{
				/*We allocate memory for data that needs to be passed to the thread. */
				THREAD_DATA *tdata = (THREAD_DATA *)malloc(sizeof(THREAD_DATA));

				/*We assign numbers for our threads. */
				tdata->thread_num = (i + 1);

				/*We want this thread to work on (n / p) elements starting from the indicated position for both input vectors. 
				The thread will compute the partial sum from the offset upto n/p elements. 
				*/

				tdata->seg_a = A + (n / p) * i;
				tdata->seg_b = B + (n / p) * i;
				/* We tell the thread the size of the segment for which partial sum needs to be completed. */
				tdata->num = n / p;

				/*Finally we spawn the thread with the above data. */

				pthread_create(&tids[i], NULL, service_thread, (void *)tdata);
			}

			/*After the threads are successfully dispatched to perform the dot product on their respective segments. We wait for all threads to finish, and finally join to the main thread.  */
			double start = clock(); //Start time!
								   /*Our main thread will wait for the threads to join. We measure this time. */
			for (int i = 0; i < p; i++)
			{
				/* Join the thread based on their thread ID. */
				pthread_join(tids[i], NULL);
			}

			double stop = clock(); // Stop time.
			double deltaT = stop - start;
			/*We print the results */

			printf("\n[*]SUCCESS: Inner product of vectors A and B successfully calculated. \n   Scroll down to see results... \n\n");

			printf("Do you want to print the randomly generated test vectors A and B? (Y / N) ");
			char c = getc(stdin);
			scanf("%c", &c);

			if (c == 'y' || c == 'Y')
			{
				print_vector('A', A, n);

				printf("\n\n");

				print_vector('B', B, n);

				printf("\n");
			}

			printf("\n[*]RESULT of A dot B = %f", total_sum);

			printf("\n\n[*]TIME ELAPSED: %f seconds.\n", deltaT / CLOCKS_PER_SEC);

			/*Deallocate heap allocated memory... */
			free(A);
			free(B);

			/* Free the array of thread IDs*/
			free(tids);
		}
		else if (op == 3)
		{
			break;
		}
		else
		{
			/*If the user provides a wrong option then we inform them with an error message... */
			printf("\nPlease provide correct option and hit ENTER ! \n");
		}
	}

	printf("\nThankyou very much for using the application..! \n");

	/*We destroy the mutex after use.*/
	pthread_mutex_destroy(&mutex);
	return 0;
}
/***** END of main() *****/

/***** Implementation of previously declared functions *****/
// Helper function that prints vectors on the terminal as a list.
// We can provide a symbol for the vector also.
void print_vector(char sym, const float *vec, int n)
{
	/*We print the symbol and opening bracket */
	printf("%c = [", sym);

	for (int i = 0; i < n; i++)
	{
		printf("%f", vec[i]);

		if (i != n - 1)
		{
			/*We print a comma to separate all our components upto before the last one. */
			printf(", ");
		}
	}

	/*Print the closing bracket. */
	printf("]");
}

/* The following function performs the vector dot product of two vectors represented as arrays of equal size. */
float vec_dot_product(const float *x, const float *y, uint32_t n)
{
	/*Variable to store the result. */
	float sum = 0;

	/*Vector dot product operation. */
	for (int i = 0; i < n; i++)
	{
		sum += x[i] * y[i];
	}
	return sum;
}

/*Finally we create the service thread function that will work as our thread that will compute the partial sum for the assigned segment of corresponding bounds in both vectors.*/

void *service_thread(void *data)
{
	/*We cast the pointer to the correct struct pointer. */
	THREAD_DATA *tdata = (THREAD_DATA *)data;
	float local_sum = 0; // Partial sum for this segment...

	/*We compute the partial sum for the segment*/

	local_sum = vec_dot_product(tdata->seg_a, tdata->seg_b, tdata->num);

	/*We finally update the total sum to calculate the final dot product of the input vectors. */
	/*We are going to work with the critical section */

	/*We grab the lock. */
	pthread_mutex_lock(&mutex);

	/*Update the critical section*/
	total_sum += local_sum;

	/*We release the lock. */

	/*We print a report on the screen of what our thread did... */
	printf("\n[*]Thread #%d calculated partial sum: %f\n", tdata->thread_num, local_sum);
	pthread_mutex_unlock(&mutex);

	/*We call free(..) on the struct pointer, because it was heap allocated at the time of thread creation*/
	free(tdata);
	return NULL;
}