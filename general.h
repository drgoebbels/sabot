#ifndef GENERAL_H_
#define GENERAL_H_

#include <stdlib.h>
#include <stdbool.h>

extern bool is_little_endian;

extern void *alloc(size_t size);
extern void *allocz(size_t size);
extern void *ralloc(void *ptr, size_t size);



#endif