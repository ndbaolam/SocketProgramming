#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <semaphore.h>

typedef long long ll;

sem_t mutex;

ll k = 10000;
ll n = 10;
ll sum = 0;

void* Calc(void *args) {
  int i = *((int *)args);
  free(args);
  printf("Thread: %d\n", i);

  ll start = i * (k / n);
  ll end = (i + 1) * (k / n) - 1;
  
  for(ll k = start; k <= end; k++) {       
    sem_wait(&mutex);
    sum += k;    
    sem_post(&mutex);
  }  
}

int main() {
  pthread_t *tid = malloc(n * sizeof(pthread_t *));
  sem_init(&mutex, 0, 1);

  for(int i = 0; i < n; i++) {
    int *arg = malloc(sizeof(int));
    *arg = i;
    pthread_create(&tid[i], NULL, Calc, arg);
  }
  for(int i = 0; i < n; i++) {
    pthread_join(tid[i], NULL);
  }
  printf("%lld\n", sum);
  sem_destroy(&mutex);  
}