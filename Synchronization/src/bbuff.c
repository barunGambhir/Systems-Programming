#include "../include/bbuff.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

//#define BUFFER_SIZE 10

typedef struct {
  int factory_number;
  double creation_ts_ms;
} candy_t;

static sem_t empty_slots;
static sem_t full_slots;

static candy_t *buffer[BUFFER_SIZE];

static sem_t mutex;

static int front = 0;
static int rear = 0;

void handle_error(char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

void bbuff_init(void) {
  int sem_es = sem_init(&empty_slots, 0, BUFFER_SIZE);
  if (sem_es != 0) {
    handle_error("sem_init failed");
  }
  int sem_fs = sem_init(&full_slots, 0, 0);
  if (sem_fs != 0) {
    handle_error("sem_init failed");
  }
  int mut = sem_init(&mutex, 0, 1);
  if (mut != 0) {
    handle_error("sem_init failed");
  }
}

void bbuff_blocking_insert(void *item) {
  int sem_es = sem_wait(&empty_slots);
  if (sem_es != 0) {
    handle_error("sem_wait failed");
  }
  int sem_mutex = sem_wait(&mutex);
  if (sem_mutex != 0) {
    handle_error("sem_wait failed");
  }
  buffer[rear] = (candy_t *)item;
  rear = (rear + 1) % BUFFER_SIZE;
  int sem_post_mutex = sem_post(&mutex);
  if (sem_post_mutex != 0) {
    handle_error("sem_post failed");
  }
  int sem_post_fs = sem_post(&full_slots);
  if (sem_post_fs != 0) {
    handle_error("sem_post failed");
  }
}

void *bbuff_blocking_extract(void) {
  int sem_fs = sem_wait(&full_slots);
  if (sem_fs != 0) {
    handle_error("sem_wait failed");
  }
  int sem_mutex = sem_wait(&mutex);
  if (sem_mutex != 0) {
    handle_error("sem_wait failed");
  }
  void *item = buffer[front];
  front = (front + 1) % BUFFER_SIZE;
  int sem_mutex_post = sem_post(&mutex);
  if (sem_mutex_post != 0) {
    handle_error("sem_post failed");
  }
  int sem_es_post = sem_post(&empty_slots);
  if (sem_es_post != 0) {
    handle_error("sem_post failed");
  }
  return item;
}

bool bbuff_is_empty(void) {
  int sem_mutex = sem_wait(&mutex);
  if (sem_mutex != 0) {
    handle_error("sem_wait failed");
  }
  int first = front;
  int last = rear;
  int sem_mutex_post = sem_post(&mutex);
  if (sem_mutex_post != 0) {
    handle_error("sem_post failed");
  }
  return (first == last);
}
