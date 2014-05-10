#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#include <sqlite3.h>
#include <unistd.h>
#include <termios.h>

#include "crypt.h"
#include "database.h"

#define PASS_FILE ".pass"

#define MAX_UNAME 20

static sqlite3 *db_handle;
static sqlite3_stmt *sql_getid;
static sqlite3_stmt *sql_insert_usr;
static sqlite3_stmt *sql_getsid;
static sqlite3_stmt *sql_insert_login;

static size_t getpassword(char *buf, size_t max);

void db_init(const char *name)
{
    int status;
    static const char getid[] = "SELECT id FROM user WHERE name=?;";
    static const char insert_usr[] = "INSERT INTO user(name) VALUES(?);";
    static const char getsid[] = "SELECT id FROM server WHERE ip=?;";
    static const char insert_login[] = "INSERT INTO login(user,handle,server,enter) VALUES(?,?,?,?);";

    status = sqlite3_open_v2(
                name,
                &db_handle,
                SQLITE_OPEN_READWRITE |
                SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_PRIVATECACHE,
                NULL);
    if(status != SQLITE_OK) {
        fprintf(stderr, "%s", sqlite3_errmsg(db_handle));
    }
    else {
        status = sqlite3_prepare(db_handle, getid, sizeof(getid)-1, &sql_getid, NULL);
        if(status != SQLITE_OK) {
            fprintf(stderr, "%s", sqlite3_errmsg(db_handle));
        }
        status = sqlite3_prepare(db_handle, insert_usr, sizeof(insert_usr)-1, &sql_insert_usr, NULL);
        if(status != SQLITE_OK) {
            fprintf(stderr, "%s", sqlite3_errmsg(db_handle));
        }
        status = sqlite3_prepare(db_handle, getsid, sizeof(getsid)-1, &sql_getsid, NULL);
        if(status != SQLITE_OK) {
            fprintf(stderr, "%s", sqlite3_errmsg(db_handle));
        }
        status = sqlite3_prepare(db_handle, insert_login, sizeof(insert_login)-1, &sql_insert_login, NULL);
        if(status != SQLITE_OK) {
            fprintf(stderr, "%s", sqlite3_errmsg(db_handle));
        }
    }
}

void add_user_record(user_s *user, char *server, time_t enter)
{
    sqlite3_int64 id;


}

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
    
    //only needed to suppress valgrind warnings
    //memset(pass, 0, sizeof(pass));
    
    puts("Enter Username");
    status = fgets(uname, MAX_UNAME, stdin);
    if(!status) {
        perror("Error Reading Username");
        exit(EXIT_FAILURE);
    }
    
    puts("Enter password");
    len = getpassword(pass, MAX_UNAME);
    
    salt = get_salt();
    s = (uint64_t *)&pass[len];
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

size_t getpassword(char *buf, size_t max)
{
    char *status;
    struct termios old, new;
    
    if (tcgetattr(fileno(stdin), &old) != 0) {
        perror("Failed to turn off echo for password");
        return 0;
    }
    new = old;
    new.c_lflag &= ~ECHO;
    if (tcsetattr(fileno(stdin), TCSAFLUSH, &new) != 0) {
        perror("Failed to turn off echo for password");
    }
    status = fgets(buf, (int)max, stdin);
    
    if(!status) {
        perror("Error Reading Password");
    }
    
    while(*status != '\n') status++;
    *status = '\0';
    
    tcsetattr(fileno(stdin), TCSAFLUSH, &old);
    return status-buf;
}

void authenticate(void)
{
    FILE *f;
    salt_s salt;
    uint64_t *s;
    size_t len;
    sha512_s hashed, attempt;
    char pass[MAX_UNAME+sizeof(salt)+1];
    
    //only needed to suppress valgrind warnings
   // memset(pass, 0, sizeof(pass));
    
    f = fopen(PASS_FILE, "r");
    if(!f) {
        perror("Error opening password file");
        return;
    }
    
    fread(&hashed.word, sizeof(hashed.word), 1, f);
    fread(&salt.whole, sizeof(salt.whole), 1, f);
    
    fclose(f);
    
    len = getpassword(pass, MAX_UNAME);
    
    s = (uint64_t *)&pass[len];
    *s = salt.whole;
    
    sha512(pass, len + sizeof(salt), &attempt);
    puts("attempted:");
    print_sha512digest(&attempt);
    puts("read in:");
    print_sha512digest(&hashed);
    putchar('\n');
    if(sha512_equal(&hashed, &attempt)) {
        puts("Authentication Passed!");
    }
    else {
        puts("Fail! ,,|,,");
    }
}
