#include "../include/stats.h"
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <pthread.h>

typedef struct{
  int factory_number;
  int candies_produced;
  int candies_consumed;
  double min_delay_ms;
  double max_delay_ms;
  double total_delay_ms;
} FactoryStats;

pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;

static int num_prod;
static FactoryStats* factory_stats;

void stats_init(int num_producers){
  num_prod = num_producers;
  factory_stats = (FactoryStats*)malloc(num_prod*sizeof(FactoryStats));
  for(int i=0; i<num_prod; i++){
    factory_stats[i].factory_number = i;
    factory_stats[i].candies_produced = 0;
    factory_stats[i].candies_consumed = 0;
    factory_stats[i].min_delay_ms = DBL_MAX;
    factory_stats[i].max_delay_ms = -1;
    factory_stats[i].total_delay_ms = 0;
  }
}

void stats_cleanup(void){
  free(factory_stats);
}

void stats_record_produced(int factory_number){
  factory_stats[factory_number].candies_produced += 1;
}

void stats_record_consumed(int factory_number, double delay_in_ms){
  pthread_mutex_lock(&stats_mutex);
  factory_stats[factory_number].candies_consumed++;
  
  if(factory_stats[factory_number].min_delay_ms == -1 || delay_in_ms < factory_stats[factory_number].min_delay_ms){
    factory_stats[factory_number].min_delay_ms = delay_in_ms;
  }

  if(delay_in_ms > factory_stats[factory_number].max_delay_ms){
    factory_stats[factory_number].max_delay_ms = delay_in_ms; 
  }

  factory_stats[factory_number].total_delay_ms += delay_in_ms;
  pthread_mutex_unlock(&stats_mutex);
}

void stats_display(void){
  printf("Statistics: \n");
  printf("%8s%10s%10s%20s%20s%20s\n", "Factory#", "#Made", "#Eaten", "Min Delay[ms]", "Avg Delay[ms]", "Max Delay[ms]");
  for(int i=0; i<num_prod; i++){
    double avg = 0;
    if(factory_stats[i].candies_consumed>0){
      avg = factory_stats[i].total_delay_ms / factory_stats[i].candies_consumed;
    }
    printf("%8d%10d%10d%20.5f%20.5f%20.5f\n", factory_stats[i].factory_number, factory_stats[i].candies_produced, factory_stats[i].candies_consumed, factory_stats[i].min_delay_ms, avg, factory_stats[i].max_delay_ms);
  }
  for(int i=0; i<num_prod; i++){
    if(factory_stats[i].candies_produced != factory_stats[i].candies_consumed){
      printf("ERROR: Mismatch between number made and eaten. \n");
      break;
    }
  }
}
