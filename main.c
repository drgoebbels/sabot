#include <stdio.h>
#include <stdlib.h>

#include "crypt.h"
#include "sanet.h"

int main(int arc, char *argv[])
{
    puts("Welcome to SA Bot");
    sha512_s digest;
    //login(S_2D_CENTRAL, "greedy.jew", "omg");
    
    sha512("bah", 3, &digest);

    
    exit(EXIT_SUCCESS);
}