#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main (){ 
   	int pid;
	int i;
	for(i = 0; i < 3; i++){
		if ((pid = fork()) < 0 ){
			printf("Sorry...cannot fork\n"); 
		}else if (pid == 0){
			printf("child %d\n", i); 
		}else{
			wait(NULL);
			printf("parent %d\n", i); 
		}
	}
	exit(0);
	return 0; 
}