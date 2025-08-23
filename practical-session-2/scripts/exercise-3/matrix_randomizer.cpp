#include <iostream>
#include <cstdlib> 
#include <ctime>
using namespace std;

const int LIMIT = 1000;

int main(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "%s use: <n_rows> <n_col>", argv[0]);
		exit(1);
	}
	int n_rows = stoi(argv[1]);
	int n_cols = stoi(argv[2]);
	srand(time(0));

	printf("%d %d\n", n_rows, n_cols);
	for (int i = 0; i < n_rows; i++) {
		for(int j = 0; j < n_cols; ++j){
			int randomNumber = (rand() % LIMIT);
			printf("%d ", randomNumber);
		}
		printf("\n");
	}
	return 0;
}
