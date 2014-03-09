#include <sqlite3.h>
#include <unistd.h>
#include <termios.h>

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "database.h"

#define MAX_UNAME 20

static void getpassword(char *buf, size_t max);

void store_account(void)
{
    char *status;
    char uname[MAX_UNAME+1], pass[MAX_UNAME+1];
    volatile uint64_t *ptr = (uint64_t *)pass;
    
    puts("Enter Username");
    status = fgets(uname, MAX_UNAME, stdin);
    if(!status) {
        perror("Error Reading Username");
        exit(EXIT_FAILURE);
    }
    puts("Enter password");
    getpassword(pass, MAX_UNAME);
    
    /* ,,|,, strict aliasing */
    ptr[0] = 0lu;
    ptr[1] = 0lu;
    ((volatile uint32_t *)ptr)[4] = 0;
}

void getpassword(char *buf, size_t max)
{
    char *status;
    struct termios old, new;
    
    if (tcgetattr(fileno(stdin), &old) != 0) {
        perror("Failed to turn off echo for password");
        return;
    }
    new = old;
    new.c_lflag &= ~ECHO;
    if (tcsetattr(fileno(stdin), TCSAFLUSH, &new) != 0) {
        perror("Failed to turn off echo for password");
    }
    status = fgets(buf, MAX_UNAME, stdin);
    if(!status) {
        perror("Error Reading Password");
    }
    tcsetattr(fileno(stdin), TCSAFLUSH, &old);
}