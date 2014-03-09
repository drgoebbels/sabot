#include "general.h"
#include <stdio.h>

void *alloc(size_t size)
{
    void *p = malloc(size);
    if(!p){
        perror("Memory Allocation Error");
        exit(EXIT_FAILURE);
    }
    return p;
}

void *allocz(size_t size)
{
    void *p = calloc(1, size);
    if(!p){
        perror("Memory Allocation Error");
        exit(EXIT_FAILURE);
    }
    return p;
}

void *ralloc(void *ptr, size_t size)
{
    void *p = realloc(ptr, size);
    if(!p){
        perror("Memory Allocation Error");
        exit(EXIT_FAILURE);
    }
    return p;
}
