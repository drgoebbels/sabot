#include <sqlite3.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

#include "database.h"

#define MAX_UNAME 20

void store_account(void)
{
    char *status, *password;
    char uname[MAX_UNAME+1];
    
    puts("Enter Username");
    status = fgets(uname, MAX_UNAME, stdin);
    if(!status) {
        perror("Error Reading Username");
        exit(EXIT_FAILURE);
    }
    password = getpass("Enter Password");
    
}
