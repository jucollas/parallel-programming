// adder.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Use: %s num1 num2 num3 ...\n", argv[0]);
    exit(1);
  }
  int n = argc - 1;
  int chunk = (n + 2) / 3;
  int fd[3][2];
  for (int i = 0; i < 3; i++) pipe(fd[i]);

  for (int p = 0; p < 3; p++) {
    pid_t pid = fork();
    if (pid == 0) {
      close(fd[p][0]);
      int sum = 0;
      for (int i = p * chunk + 1; i <= ( p + 1 ) * chunk && i <= n; i++) {
        sum += atoi(argv[i]);
      }
      write(fd[p][1], &sum, sizeof(sum));
      close(fd[p][1]);
      exit(0);
    }
  }
  int ans = 0, part;
  for (int p = 0; p < 3; p++) {
    close(fd[p][1]);
    read(fd[p][0], &part, sizeof(part));
    ans += part;
    close(fd[p][0]);
  }
  for (int i = 0; i < 3; i++) wait(NULL);
  printf("%d\n", ans);
  return 0;
}
