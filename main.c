#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "crypt.h"
#include "sanet.h"
#include "database.h"

#define N_TESTS 1000

int main(int arc, char *argv[])
{
    uint64_t accum = 0;
    uint32_t i;
    clock_t time;
    
    puts("Welcome to SA Bot");
    sha512_s digest;
    //login(S_2D_CENTRAL, "greedy.jew", "omg");
    
    sha512(argv[1], strlen(argv[1]), &digest);

    print_digest(&digest);
    //store_account();
    //authenticate();
    /*for(i = 0; i < N_TESTS; i++) {
        time = clock();
        sha512(argv[1], strlen(argv[1]), &digest);
        accum += (clock() - time);
    }
    
    print_digest(&digest);*/
   // printf("\naverage time: %f ticks\n", accum/(double)N_TESTS);
    
    exit(EXIT_SUCCESS);
}
