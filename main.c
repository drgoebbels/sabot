#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "crypt.h"
#include "sanet.h"
#include "database.h"

#define N_TESTS 1000

char *key = "\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf\x4f\x3c";
char *clear = "\x32\x43\xf6\xa8\x88\x5a\x30\x8d\x31\x31\x98\xa2\xe0\x37\x07\x34";

int main(int arc, char *argv[])
{
    uint64_t accum = 0;
    uint32_t i, j;
    clock_t time;
    
    aes_encrypt(clear, sizeof(clear)-1, key);
    
    return 0;
    
    
    
    
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
