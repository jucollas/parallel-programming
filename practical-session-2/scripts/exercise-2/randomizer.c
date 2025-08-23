#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const int LIMIT = 10;

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Use: %s n\n", argv[0]);
		exit(1);
	}

	int n = atoi(argv[1]);
	srand(time(NULL));
	printf("%d ", n);
	for (int i = 0; i < n; i++) {
		printf("%d ", rand() % LIMIT);
	}
	printf("\n");
	return 0;
}
