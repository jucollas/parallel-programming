#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

int N_THREADS;  

typedef struct {
	int *a;        
	int *temp;     
	int n;         
	int start;     
	int end;       
} tdata;

void* thread_count_sort(void* arg) {
	tdata* data = (tdata*) arg;
	int *a = data->a;
	int *temp = data->temp;
	int n = data->n;
	for (int i = data->start; i < data->end; i++) {
		int count = 0;
		for (int j = 0; j < n; j++) {
			if (a[j] < a[i])
				count++;
			else if (a[j] == a[i] && j < i)
				count++;
		}
		temp[count] = a[i];
	}
	return NULL;
}

void count_sort_parallel(int a[], int n) {
	int num_threads = N_THREADS < n ? N_THREADS : n;
	pthread_t threads[num_threads];
	tdata thread_data[num_threads];

	int *temp = malloc(n * sizeof(int));
	if (temp == NULL) {
		perror("malloc failed");
		exit(1);
	}

	int chunk = n / num_threads;
	int remainder = n % num_threads;

	int start = 0;
	for (int t = 0; t < num_threads; t++) {
		int end = start + chunk + (t < remainder ? 1 : 0);
		thread_data[t].a = a;
		thread_data[t].temp = temp;
		thread_data[t].n = n;
		thread_data[t].start = start;
		thread_data[t].end = end;
		pthread_create(&threads[t], NULL, thread_count_sort, &thread_data[t]);
		start = end;
	}

	for (int i = 0; i < num_threads; i++)
		pthread_join(threads[i], NULL);

	memcpy(a, temp, n * sizeof(int));
	free(temp);
}

void print_array(int a[], int n) {
	for (int i = 0; i < n; i++)
		printf("%d ", a[i]);
	printf("\n");
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Use: %s <num_thread>\n", argv[0]);
		exit(1);
	}

	N_THREADS = atoi(argv[1]); 

	int n; 
	scanf("%d", &n);

	int *arr = malloc(n * sizeof(int));
	if (!arr) {
		perror("malloc failed");
		exit(1);
	}

	for (int i = 0; i < n; ++i) {
		scanf("%d", &arr[i]);
	}

	print_array(arr, n);
	count_sort_parallel(arr, n);
	print_array(arr, n);

	free(arr);
	return 0;
}
