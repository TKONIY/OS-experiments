#include <stdio.h>
#define __USE_GNU
#include <errno.h>
#include <sched.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

int shared_var = 0;

int mythread(void* params) {
  shared_var = 1000;  //在此线程内修改全局变量
  printf("子线程中shared_var=%d\n", shared_var);
  return 0;
}

int main() {
  //必须动态分配栈
  const int STACK_SIZE = 65536;
  char* child_stack = malloc(STACK_SIZE);
  if (!child_stack) return -1;

  pid_t pid = clone(mythread, child_stack + STACK_SIZE,
                    CLONE_VM | CLONE_FS | CLONE_FILES | SIGCHLD,
                    NULL);  // SIGCHLD,发送信号给父进程
  wait(NULL);

  printf("主线程中shared_var=%d\n", shared_var);
  free(child_stack);
  return 0;
}