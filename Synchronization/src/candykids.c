#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include "../include/bbuff.h"
#include "../include/stats.h"


typedef struct {
  int factory_number;
  double creation_ts_ms;
} candy_t;

double current_time_in_ms(void){
  struct timespec now;
  clock_gettime(CLOCK_REALTIME, &now);
  return now.tv_sec * 1000.0 + now.tv_nsec / 1000000.0;
}

void *factory_thread(void *arg){
  int factory_number = *(int *)arg;
  while(1){
    int wait_time = rand()%4;
    printf("# %d factory ships candy and waits for %ds\n", factory_number, wait_time);
  
    candy_t *new_candy = (candy_t *)malloc(sizeof(candy_t));
    new_candy->factory_number = factory_number;
    new_candy->creation_ts_ms = current_time_in_ms();

    bbuff_blocking_insert(new_candy);
    stats_record_produced(factory_number);

    sleep(wait_time);
  }
  pthread_exit(NULL);
}

void *kid_thread(void *arg){
  while(1){
    candy_t *candy = (candy_t *)bbuff_blocking_extract();
    double delay = current_time_in_ms() - candy->creation_ts_ms;
    printf("Kid consumed candy from # %d factory and waited for %.2fms\n", candy->factory_number, delay);
    stats_record_consumed(candy->factory_number, delay);
    free(candy);
    int wait_time = rand() % 2;
    sleep(wait_time);
  }
  pthread_exit(NULL);
}

int main(int argc, char *argv[]){
  if (argc !=4 ){
    handle_error("Please add exactly three numeric arguments");
  }
  
  int num_factories = atoi(argv[1]) ;
  int num_kids = atoi(argv[2]);
  int seconds_to_run = atoi(argv[3]);
  
  if(num_factories <= 0 || num_kids <= 0 || seconds_to_run <= 0){
    handle_error("Invalid argumants. Must be greater than 0");
  }

  srand(time(NULL));
  bbuff_init();
  stats_init(num_factories);
  
  pthread_t factory_threads[num_factories];
  pthread_t kid_threads[num_kids];
  int factory_numbers[num_factories]; 

  for(int i=0; i<num_factories; i++){
    factory_numbers[i] = i;
    int p_create = pthread_create(&factory_threads[i], NULL, factory_thread, &factory_numbers[i]);
    if(p_create!=0){
      handle_error("pthread_create failed");
    }
  }

  for(int i=0; i<num_kids; i++){
    int p_create = pthread_create(&kid_threads[i], NULL, kid_thread, NULL);
    if(p_create!=0){
      handle_error("pthread_create failed");
    }
  }

  for(int i=0; i<seconds_to_run; i++){
    sleep(1);
    printf("Time %ds\n", i+1);
  }

  for(int i=0; i<num_factories; i++){
    int p_cancel = pthread_cancel(factory_threads[i]);
    if(p_cancel!=0){
      handle_error("pthread_cancel failed");
    }
    int p_join = pthread_join(factory_threads[i], NULL);
    if(p_join!=0){
      handle_error("pthread_join failed");
    }
  }

  while(!bbuff_is_empty()){
    printf("Waiting for all candies to be consumed");
    sleep(1);
  }

  for(int i=0; i<num_kids; i++){
    int p_cancel = pthread_cancel(kid_threads[i]);
    if(p_cancel!=0){
      handle_error("pthread_cancel failed");
    }
    int p_join = pthread_join(kid_threads[i], NULL);
    if(p_join!=0){
      handle_error("pthread_join failed");
    }
  }

  stats_display();
  stats_cleanup();

  return EXIT_SUCCESS;;
}
