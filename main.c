#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "crypt.h"
#include "sanet.h"
#include "database.h"
#include "regex.h"

#define N_TESTS 1000

char *key = "\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf\x4f\x3c";
char clear[] = "\x32\x43\xf6\xa8\x88\x5a\x30\x8d\x31\x31\x98\xa2\xe0\x37\x07\x34\x34\x34\x34\x34\x34\x34\x34\x34\x34\x34\x34\xab\xab\xab\xab\xab";

int main(int arc, char *argv[])
{
    aes_digest_s *tmp;
    
    compile_regex("^(a|bc)*(hello(bob|(a|b)))?[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4,5,5,5     ,5}$");
    
    return 0;
    print_aesdigest(tmp = aes_encrypt(clear, sizeof(clear)-1, key));
    puts("\n");
    print_aesdigest(aes_decrypt(tmp->data, tmp->size, key));
    
    return 0;
    
    
    
    
    puts("Welcome to SA Bot");
    sha512_s digest;
    
    sha512(argv[1], strlen(argv[1]), &digest);

    print_sha512digest(&digest);
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
