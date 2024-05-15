#ifndef BBUFF_H
#define BBUFF_H
#include <stdbool.h>
#define BUFFER_SIZE 10

void handle_error(char* msg);

void bbuff_init(void);
void bbuff_blocking_insert(void* item);
void *bbuff_blocking_extract(void);
bool bbuff_is_empty(void);

#endif
