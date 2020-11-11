#include <memory.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_SIZE 1000
static int tmp[MAX_SIZE]; /*临时数组*/

void Merge(int* S, int low, int mid, int high) {
  /*merge*/
  int i = low, j = mid + 1, k = low;
  while (i <= mid && j <= high) {
    if (S[i] < S[j])
      tmp[k++] = S[i++];
    else
      tmp[k++] = S[j++];
  }
  while (i <= mid) tmp[k++] = S[i++];
  while (j <= high) tmp[k++] = S[j++];
  memcpy(S + low, tmp + low, sizeof(int) * (high - low + 1));
}

typedef struct {
  int* S;
  int low;
  int high;
} MSortParams;

void* MSort(void* params) {
  MSortParams* p = params;
  int* S = p->S;
  int low = p->low;
  int high = p->high;

  if (low >= high) return NULL;
  int mid = (low + high) / 2;

  /* merge Sort left size*/
  MSortParams msp_l = {S, low, mid};
  pthread_t pid_l;
  pthread_create(&pid_l, NULL, MSort, &msp_l);

  MSortParams msp_r = {S, mid + 1, high};
  pthread_t pid_r;
  pthread_create(&pid_r, NULL, MSort, &msp_r);

  pthread_join(pid_l, NULL);
  pthread_join(pid_r, NULL);

  Merge(S, low, mid, high);
}

int main() {
  clock_t t1 = clock();
  int a[1000], b[1000];
  for (int i = 0; i < 1000; ++i) a[i] = b[i] = rand() % 10000;

  MSortParams msp1 = {a, 0, sizeof(a) / sizeof(int) - 1};

  pthread_t pid;
  pthread_create(&pid, NULL, MSort, &msp1);
  pthread_join(pid, NULL);

  // for (int i = 0; i < sizeof(a) / sizeof(int); ++i) {
  //   printf("%d\n", a[i]);
  // }
  printf("t1 = %ld\n", clock() - t1);

  clock_t t2;
  MSortParams msp2 = {b, 0, sizeof(b) / sizeof(int) - 1};
  MSort(&msp2);
  printf("t2 = %ld\n", clock() - t2);
}