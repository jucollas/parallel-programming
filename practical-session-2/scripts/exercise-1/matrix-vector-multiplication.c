#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int thread_count;   
int m, n;           
double **A;         
double *x;          
double *y;          

void *Pth_mat_vect(void *rank) {
	long my_rank = (long) rank;
	int local_m = m / thread_count;
	int my_first_row = my_rank * local_m;
	int my_last_row  = (my_rank + 1) * local_m - 1;
	for (int i = my_first_row; i <= my_last_row; i++) {
		y[i] = 0.0;
		for (int j = 0; j < n; j++)
			y[i] += A[i][j] * x[j];
	}
	return NULL;
}

void mult_matrix_vector(){
	pthread_t *threads = malloc(thread_count * sizeof(pthread_t));
	for (long i = 0; i < thread_count; i++)
		pthread_create(&threads[i], NULL, Pth_mat_vect, (void *) i);

	for (long i = 0; i < thread_count; i++)
		pthread_join(threads[i], NULL);
	free(threads);
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Use: %s <num_thread>\n", argv[0]);
		exit(1);
	}

	thread_count = atoi(argv[1]);

	printf("Input m y n: ");
	scanf("%d %d", &m, &n);

	A = malloc(m * sizeof(double *));
	for (int i = 0; i < m; i++)
		A[i] = malloc(n * sizeof(double));

	x = malloc(n * sizeof(double));
	y = malloc(m * sizeof(double));

	printf("Input A (%d x %d):\n", m, n);
	for (int i = 0; i < m; i++)
		for (int j = 0; j < n; j++)
			scanf("%lf", &A[i][j]);

	printf("Input x (%d):\n", n);
	for (int j = 0; j < n; j++)
		scanf("%lf", &x[j]);

	mult_matrix_vector();

	printf("Answer: y = A * x:\n");
	for (int i = 0; i < m; i++)
		printf("%lf\n", y[i]);

	for (int i = 0; i < m; i++) free(A[i]);
	free(A); free(x); free(y); 
	return 0;
}


