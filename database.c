#include <sqlite3.h>
#include <unistd.h>
#include <termios.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#include "crypt.h"
#include "database.h"

#define PASS_FILE ".pass"

#define MAX_UNAME 20

static void getpassword(char *buf, size_t max);


void store_account(void)
{
    uint64_t *s;
    size_t len;
    char *status;
    salt_s salt;
    FILE *f;
    sha512_s digest;
    char uname[MAX_UNAME+sizeof(salt)+1], pass[MAX_UNAME+1];
    volatile uint64_t *ptr = (uint64_t *)pass;
    
    memset(pass, 0, sizeof(pass));
    
    
    
    puts("Enter Username");
    status = fgets(uname, MAX_UNAME, stdin);
    if(!status) {
        perror("Error Reading Username");
        exit(EXIT_FAILURE);
    }
    
    
    puts("Enter password");
    getpassword(pass, MAX_UNAME);
    len = strlen(pass);
    
    //printf("%ld\n", len);
    
    salt = get_salt();
    s = (uint64_t *)&pass[len-1];
    *s = salt.whole;
    
    sha512(pass, len + sizeof(uint64_t), &digest);

    /* ,,|,, strict aliasing */
    ptr[0] = 0lu;
    ptr[1] = 0lu;
    ((volatile uint32_t *)ptr)[4] = 0;
    
    f = fopen(PASS_FILE, "w");
    if(!f) {
        perror("Error Storing Account");
        return;
    }
    fwrite(digest.word, sizeof(digest.word), 1, f);
    fwrite(&salt, sizeof(salt), 1, f);
    fclose(f);
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

void authenticate(void)
{
    FILE *f;
    salt_s salt;
    uint64_t *s;
    size_t len;
    sha512_s hashed, attempt;
    char pass[MAX_UNAME+sizeof(salt)+1];
    
    f = fopen(PASS_FILE, "r");
    if(!f) {
        perror("Error opening password file");
        return;
    }
    
    fread(&hashed.word, sizeof(hashed.word), 1, f);
    fread(&salt.whole, sizeof(salt.whole), 1, f);
    
    fclose(f);
    
    getpassword(pass, MAX_UNAME);
    len = strlen(pass);
    
    //printf("%ld\n",len);
    
    s = (uint64_t *)&pass[len-1];
    *s = salt.whole;
    
    sha512(pass, len + sizeof(salt), &attempt);
    puts("attempted:");
    print_digest(&attempt);
    puts("read in:");
    print_digest(&hashed);
    putchar('\n');
    if(sha512_equal(&hashed, &attempt)) {
        puts("Authentication Passed!");
    }
    else {
        puts("Fail! ,,|,,");
    }
}
