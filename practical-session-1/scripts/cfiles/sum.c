// sum.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char const *argv[]){
  if (argc < 3){
    printf("Using <start> <end>\n");
    return 1;
  }
  int start = atoi(argv[1]);
  int end = atoi(argv[2]);
  int fd[2];
  pid_t pid;

  // Create pipe
  if(pipe(fd) == -1){
    perror("Error: Creating the pipe");
    exit(1);
  }

  pid = fork();

  if(pid < 0){
    perror("Error: fork");
    exit(1);
  }

  if(pid == 0){
    // Process child
    close(fd[0]); // close read, only write
    int ans = 0;
    for(int i = start; i <= end; ++i){
      ans += i;
    }
    // Send parent
    write(fd[1], &ans, sizeof(ans));
    close(fd[1]); // close write
    exit(0);
  }else {
    // Process parent
    close(fd[1]); // close write, only read
    int ans;
    wait(NULL);

    read(fd[0], &ans, sizeof(ans));
    close(fd[0]);
    printf("Parent: The sum calculated for the child is %d\n", ans);
  }
  return 0;
}