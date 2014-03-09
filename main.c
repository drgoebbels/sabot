#include <stdio.h>
#include <stdlib.h>

#include "crypt.h"
#include "sanet.h"
#include "database.h"

int main(int arc, char *argv[])
{
   
    
    
    puts("Welcome to SA Bot");
    sha512_s digest;
    //login(S_2D_CENTRAL, "greedy.jew", "omg");
    
    
    store_account();
    
    sha512("bah", 3, &digest);

    
    exit(EXIT_SUCCESS);
}
