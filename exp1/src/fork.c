#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

int shared_var = 0;  //需要验证的共享变量

int main() {
  pid_t pid = fork();
  if (pid < 0) {
    perror("failed to fork()");
    exit(-1);
  } else if (pid == 0) {
    shared_var = 1000;  // 子进程修改全局变量
    printf("子进程中shared_var为%d\n", shared_var);
  }else {
    wait(NULL);
    printf("主进程中shared_var为%d\n", shared_var);
  }
  return 0;
}