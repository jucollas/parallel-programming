#include <stdio.h>
#include <unistd.h>

int main (void){
	printf("Line 1 ..\n");
	write(1,"Line 2 ",7);
}