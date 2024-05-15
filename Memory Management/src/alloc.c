#include "../include/alloc.h"
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define HEADER_SIZE sizeof(struct header)

// header as defined in alloc.h
/*struct header{
  uint64_t size;
  struct header* next;
};*/

static enum algs allocation_algo = FIRST_FIT; // setting a default value
static int heap_limit = -1;

static struct header *free_list = NULL;

static int total_allocated_size = 0;

void *alloc(int size) {
  //check for invalid size 
  if(size<=0 || total_allocated_size + size > heap_limit){
    return NULL;
  }
  size += HEADER_SIZE;
  struct header **current_ptr = &free_list;
  struct header *allocated_block = NULL;

  switch(allocation_algo){

    case FIRST_FIT: {
      while(*current_ptr != NULL){
        if((*current_ptr)->size >= size){
          allocated_block = *current_ptr;
          *current_ptr = allocated_block->next;
          break;
        }
        current_ptr = &((*current_ptr)->next);
      }
      break;
    }
    
    case BEST_FIT: {
      struct header **best_fit_ptr = NULL;
      int best_fit_diff = -1;
      while(*current_ptr != NULL){
        if((*current_ptr)->size >= size){
          int diff = (*current_ptr)->size - size;
          if(best_fit_ptr == NULL || diff < best_fit_diff){
            best_fit_ptr = current_ptr;
            best_fit_diff = diff;
          }
        }
        current_ptr = &((*current_ptr)->next);
      }
      if(best_fit_ptr != NULL){
        allocated_block = *best_fit_ptr;
        *best_fit_ptr = allocated_block->next;
      }
      break;
    }

    case WORST_FIT: {
      struct header **worst_fit_ptr = NULL;
      int worst_fit_diff = -1;
      while(*current_ptr != NULL){
        if((*current_ptr)->size >= size){
          int diff = (*current_ptr)->size - size;
          if(worst_fit_ptr == NULL || diff > worst_fit_diff){
            worst_fit_ptr = current_ptr;
            worst_fit_diff = diff;
          }
        }
        current_ptr = &((*current_ptr)->next);
      }
      if(worst_fit_ptr != NULL){
        allocated_block = *worst_fit_ptr;
        *worst_fit_ptr = allocated_block->next;
      }
      break;
    }
  }

  //no suitable block is found :(
  if(allocated_block == NULL){
    int increment_size = (size > INCREMENT) ? size : INCREMENT;
    if(total_allocated_size + increment_size > heap_limit){
      return NULL;
    }
    struct header *new_block = sbrk(increment_size);
    if(new_block == (void *)-1){
      return NULL;
    }
    new_block->size = increment_size - HEADER_SIZE;
    new_block->next = NULL;
    allocated_block = new_block;
  }

  //split the block if possible
  if(allocated_block->size > size){
    struct header * remaining_block = (struct header *)((char *)allocated_block + size);
    remaining_block->size = allocated_block->size - size;
    remaining_block->next = allocated_block->next;
    allocated_block->size = size;
    allocated_block->next = NULL;
    current_ptr = &free_list;
    while(*current_ptr != NULL && *current_ptr < remaining_block){
      current_ptr = &((*current_ptr)->next);
    }
    remaining_block->next = *current_ptr;
    *current_ptr = remaining_block;
  }

  total_allocated_size+=allocated_block->size;
  return (void *)((char *)allocated_block + HEADER_SIZE);

}




void dealloc(void *ptr) {
  if (ptr == NULL) {
    return;
  }
  struct header *block = (struct header *)((char *)ptr - HEADER_SIZE);
  struct header **curr = &free_list;

  while (*curr != NULL && *curr < block) {
    curr = &((*curr)->next);
  }

  block->next = *curr;
  *curr = block;
  struct header *curr_block = free_list;
  
  while(curr_block != NULL){
    struct header *next_block = curr_block->next;
    if(next_block != NULL && 
        (char *)curr_block + curr_block->size == (char *)next_block){
      curr_block->size += next_block->size;
      curr_block->next = next_block->next;
    } 
    else{
      curr_block = next_block;  
    }
  }

  total_allocated_size -= block->size;

}

void allocopt(enum algs algorithm, int limit) {
  allocation_algo = algorithm;
  heap_limit = limit;
  free_list = NULL;
  total_allocated_size = 0;
  sbrk(-((intptr_t)total_allocated_size));  
}

struct allocinfo allocinfo(void) {
  struct allocinfo info;
  info.free_size = 0;
  info.free_chunks = 0;
  info.largest_free_chunk_size = 0;
  info.smallest_free_chunk_size = INT_MAX;

  for (struct header *curr = free_list; curr; curr = curr->next) {
    info.free_size += curr->size;
    info.free_chunks++;
    if (curr->size > info.largest_free_chunk_size) {
      info.largest_free_chunk_size = curr->size;
    }
    if (info.smallest_free_chunk_size == 0 || curr->size < info.smallest_free_chunk_size) {
      info.smallest_free_chunk_size = curr->size;
    }
  }
  return info;
}
