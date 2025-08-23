#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

int thread_count;
double a, b;   
int n;         
double h;
double global_result = 0.0;
pthread_mutex_t mutex;

double f(double x) {
	return x*x;
}

void *Trap(void *rank) {
	long my_rank = (long) rank;
	int local_n = n / thread_count;   
	double local_a = a + my_rank * local_n * h;
	double local_b = local_a + local_n * h;
	double local_sum = (f(local_a) + f(local_b)) / 2.0;

	for (int i = 1; i <= local_n - 1; i++) {
		double x = local_a + i * h;
		local_sum += f(x);
	}
	local_sum *= h;

	pthread_mutex_lock(&mutex);
	global_result += local_sum;
	pthread_mutex_unlock(&mutex);
	return NULL;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Use: %s <num_thread>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	thread_count = atoi(argv[1]);

	a = 0.0; 
	b = 12.0; 
	n = 1000000;
	h = (b - a) / n;

	pthread_t *thread_handles = malloc(thread_count * sizeof(pthread_t));
	pthread_mutex_init(&mutex, NULL);

	for (long thread = 0; thread < thread_count; thread++)
		pthread_create(&thread_handles[thread], NULL, Trap, (void*) thread);

	for (long thread = 0; thread < thread_count; thread++)
		pthread_join(thread_handles[thread], NULL);

	printf("Approximate integral in [%f, %f] = %.15f\n", a, b, global_result);

	pthread_mutex_destroy(&mutex);
	free(thread_handles);
	return 0;
}
