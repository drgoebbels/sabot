#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "crypt.h"
#include "sanet.h"
#include "database.h"

int main(int arc, char *argv[])
{
   
    
    
    puts("Welcome to SA Bot");
    sha512_s digest;
    //login(S_2D_CENTRAL, "greedy.jew", "omg");
    
    
    sha512(argv[1], strlen(argv[1]), &digest);
    print_digest(&digest);
    
    exit(EXIT_SUCCESS);
}
