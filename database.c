#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#include <sqlite3.h>
#include <unistd.h>
#include <termios.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <semaphore.h>

#include "crypt.h"
#include "database.h"

#define PASS_FILE ".pass"

#define MAX_UNAME 20

static sqlite3 *db_handle;
static sqlite3_stmt *sql_getid;
static sqlite3_stmt *sql_insert_usr;
static sqlite3_stmt *sql_getsid;
static sqlite3_stmt *sql_insert_login;

static sem_t *sql_db_lock;

static size_t getpassword(char *buf, size_t max);

void db_init(const char *name)
{
    int status;

    static const char getid[] = "SELECT id FROM user WHERE name=?;";
    static const char insert_usr[] = "INSERT INTO user(name) VALUES(?);";
    static const char getsid[] = "SELECT id FROM server WHERE ip=?;";
    static const char insert_login[] = "INSERT INTO login(user,handle,server,enter) VALUES(?,?,?,?);";
   // static const char insert_msg[] = "INSERT INTO message(message,type,message_to,message_from,flag"

    if(!db_handle) {
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
            sql_db_lock = sem_open("sadb_mutex", O_CREAT, S_IRUSR | S_IWUSR, 1);
            if(sql_db_lock == SEM_FAILED) {
                perror("Failure to obtain db mutex.");
                return;
            }

            status = sqlite3_prepare_v2(db_handle, getid, sizeof(getid), &sql_getid, NULL);
            if(status != SQLITE_OK) {
                fprintf(stderr, "%s", sqlite3_errmsg(db_handle));
            }
            status = sqlite3_prepare_v2(db_handle, insert_usr, sizeof(insert_usr), &sql_insert_usr, NULL);
            if(status != SQLITE_OK) {
                fprintf(stderr, "%s", sqlite3_errmsg(db_handle));
            }
            status = sqlite3_prepare_v2(db_handle, getsid, sizeof(getsid), &sql_getsid, NULL);
            if(status != SQLITE_OK) {
                fprintf(stderr, "%s", sqlite3_errmsg(db_handle));
            }
            status = sqlite3_prepare_v2(db_handle, insert_login, sizeof(insert_login), &sql_insert_login, NULL);
            if(status != SQLITE_OK) {
                fprintf(stderr, "%s", sqlite3_errmsg(db_handle));
            }
        }
        atexit(db_clean);
    }
}

void add_user_record(user_s *user, char *server, time_t enter)
{
    sqlite3_int64 id = -1, sid = -1;
    int status, type, icol = 0;
    size_t len = strlen(user->name);

    //sem_wait()
    status = sqlite3_bind_text(sql_getid, 1, user->name, len, SQLITE_STATIC);
    if(status == SQLITE_OK) {
        while((status = sqlite3_step(sql_getid)) != SQLITE_DONE) {
            if(status == SQLITE_ROW) {
                type = sqlite3_column_type(sql_getid, icol);
                if(type == SQLITE_INTEGER)
                    id = sqlite3_column_int64(sql_getid, icol);
                icol++;
            }
            else if (status == SQLITE_ERROR) {
                fprintf(stderr, "%s", sqlite3_errmsg(db_handle));
                break;
            }
            else {
                //...
            }
        }
        sqlite3_reset(sql_getid);
        sqlite3_clear_bindings(sql_getid);

        if(id == -1) {
            status = sqlite3_bind_text(sql_insert_usr, 1, user->name, len, SQLITE_STATIC);
            if(status == SQLITE_OK) {
                status = sqlite3_step(sql_insert_usr);
                if(status != SQLITE_DONE) {
                    fprintf(stderr, "%s", sqlite3_errmsg(db_handle));
                    return;
                }
                id = sqlite3_last_insert_rowid(db_handle);
                sqlite3_reset(sql_insert_usr);
                sqlite3_clear_bindings(sql_insert_usr);
            }
        }
        status = sqlite3_bind_text(sql_getsid, 1, server, strlen(server), SQLITE_STATIC);
        if(status == SQLITE_OK) {
            icol = 0;
            while((status = sqlite3_step(sql_getsid)) != SQLITE_DONE) {
                if(status == SQLITE_ROW) {
                    type = sqlite3_column_type(sql_getsid, icol);
                    if(type == SQLITE_INTEGER)
                        sid = sqlite3_column_int64(sql_getsid, icol);
                    icol++;
                }
                else if (status == SQLITE_ERROR) {
                    fprintf(stderr, "%s", sqlite3_errmsg(db_handle));
                }
                else {
                    //printf("type: %d\n",)
                    //...
                }
            }
        }
        if(sid > 0) {
            sqlite3_bind_int64(sql_insert_login, 1, id);
            sqlite3_bind_text(sql_insert_login, 2, user->id, 3, SQLITE_STATIC);
            sqlite3_bind_int64(sql_insert_login, 3, sid);
            sqlite3_bind_int(sql_insert_login, 4, (int)enter);
            status = sqlite3_step(sql_insert_login);
            if(status != SQLITE_DONE) {
                fprintf(stderr, "%s", sqlite3_errmsg(db_handle));
            }
            sqlite3_reset(sql_insert_login);
            sqlite3_clear_bindings(sql_insert_login);
        }
        else {
            perror("Database Error reading server primary key");
        }
        sqlite3_reset(sql_getsid);
        sqlite3_clear_bindings(sql_getsid);
    }
    else {
        fprintf(stderr, "%s", sqlite3_errmsg(db_handle));
    }
    sqlite3_reset(sql_getid);
    sqlite3_clear_bindings(sql_getid);
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

inline void db_clean(void)
{
    sem_unlink("sadb_mutex");
    sqlite3_close(db_handle);
}
