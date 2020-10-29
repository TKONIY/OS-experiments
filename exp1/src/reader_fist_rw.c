/**
 * 打印一行\nwrite\n模拟写动作, 打印一个r模拟读动作,通过输出查看是否冲突
 * 如果write没有成为单独一行则表示程序没锁住
 * 不知道怎么检查读写优先,可以读者进入一个循环的时候打印一下,看看在这之后有没有写者进行write
 */
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

sem_t wrt, mutex; /*读写锁和readcount的锁,初始化为1*/
int readcount;

void* writer(void* params) {
  while (1) {

    sem_wait(&wrt);
    printf("\nwrite\n"); /*writing*/
    sem_post(&wrt);

  }
}

void* reader(void* params) {
  while (1) {

    sem_wait(&mutex);
    readcount++;
    if (readcount == 1) sem_wait(&wrt);
    sem_post(&mutex);

    printf("r"); /*reading*/

    sem_wait(&mutex);
    readcount--;
    if (readcount == 0) sem_post(&wrt);
    sem_post(&mutex);

  }
}

int main() {
  /*init*/
  sem_init(&wrt, 0, 1);
  sem_init(&mutex, 0, 1);
  readcount = 0;

  /*create 5 reader and 10 writer*/
  pthread_t pid;
  for (int i = 0; i < 5; ++i) pthread_create(&pid, NULL, writer, NULL);
  for (int i = 0; i < 10; ++i) pthread_create(&pid, NULL, reader, NULL);
  pthread_join(pid, NULL); /*run forever*/
}