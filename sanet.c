#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "general.h"
#include "sanet.h"

#define SLEEP_TIME  20
#define LOGIN_FLAG "09"

#define MAX_UNAME_PASS 20

connect_inst_s *connlist;
monitor_s monitor = {
    .inuse = 0,
    .cond = PTHREAD_COND_INITIALIZER,
    .lock = PTHREAD_MUTEX_INITIALIZER
};

static connect_inst_s *connlist_tail;

/* Initial Packet Sent when logging in */
static char init_send[] = {
    0x30, 0x38, 0x48, 0x78, 0x4f, 0x39, 0x54, 0x64,
    0x43, 0x43, 0x36, 0x32, 0x4e, 0x77, 0x6c, 0x6e,
    0x31, 0x50, 0x00
};

/* General ACK 0 */
static char ack_x0[] = {
    0x30, 0x00
};

/* General ACK 1 */
static char ack_x1[] = {
    0x30, 0x63, 0x00
};

/* General ACK 2 */
static char ack_x2[] = {
    0x30, 0x31, 0x00
};

/* This is sent to complete a login */
static char finish_login[] = {
    0x30, 0x33, 0x5f, 0x00
};

static int socket_(const char *server);
static void connect_thread(connect_inst_s *c);
static void ack_thread(connect_inst_s *c);
static void add_connection(connect_inst_s *c);

static inline int netgetchar(connect_inst_s *conn);
static inline char *nexttoken(connect_inst_s *conn);
static inline bool is_namechar(int c);
static inline bool is_gamenamechar(int c);

int socket_(const char *server)
{
    int sock;
    static struct sockaddr_in sockin;
    static int res;
    
    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < -1) {
        perror("cannot create socket");
        exit(EXIT_FAILURE);
    }
    printf("socket number: %d", sock);
    
    memset(&sockin, 0, sizeof(sockin));
    
    sockin.sin_family = AF_INET;
    sockin.sin_port = htons(PORT);
    res = inet_pton(AF_INET, server, &sockin.sin_addr);
    
    if (0 > res) {
        perror("error: first parameter is not a valid address family");
        close(sock);
        exit(EXIT_FAILURE);
    }
    else if (res == 0) {
        perror("char string (second parameter does not contain valid ipaddress)");
        close(sock);
        exit(EXIT_FAILURE);
    }
    
    if (connect(sock, (struct sockaddr *)&sockin, sizeof(sockin)) == -1) {
        perror("connect failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
    return sock;
}

connect_inst_s *login(const char *server, const char *uname, const char *pass)
{
    int sock;
    char buf[2*MAX_UNAME_PASS+5];
    connect_inst_s *conn;
    
    if(strlen(uname) + strlen(pass) + sizeof(LOGIN_FLAG) + 1 > 2*MAX_UNAME_PASS+5) {
        fprintf(stderr, "Input too large you retard\n");
        return NULL;
    }
    
    sock = socket_(server);
    
    send(sock, init_send, sizeof(init_send), 0);
    recv(sock, buf, sizeof(buf), 0);
    
    sprintf(buf, LOGIN_FLAG "%s;%s", uname, pass);
    send(sock, buf, strlen(buf)+1, 0);
    
    recv(sock, buf, sizeof(buf), 0);

    send(sock, ack_x1, sizeof(ack_x1), 0);
    send(sock, ack_x2, sizeof(ack_x2), 0);
    send(sock, finish_login, sizeof(finish_login), 0);
    recv(sock, buf, sizeof(buf), 0);
    
    conn = alloc(sizeof(*conn));
    conn->server = server;
    conn->sock = sock;
    conn->chat.head = NULL;
    conn->chat.tail = NULL;
    conn->i = 0;
    conn->len = 0;
    pthread_mutex_init(&conn->chat.lock, NULL);
        
    add_connection(conn);
    
    pthread_create(&conn->thread, NULL, (void *(*)(void *))connect_thread, conn);
    pthread_create(&conn->thread, NULL, (void *(*)(void *))ack_thread, conn);

    return conn;
}

void connect_thread(connect_inst_s *con)
{
    int c;
    uint64_t count = 0;
    int sock = con->sock;
    chat_packet_s *packet;
    
    while(1) {
        nexttoken(con);
        if(monitor.inuse) {
            pthread_mutex_lock(&monitor.lock);
            pthread_cond_signal(&monitor.cond);
            pthread_mutex_unlock(&monitor.lock);
        }
        count++;
    }
}

void ack_thread(connect_inst_s *c)
{
    int sock = c->sock;
    pthread_mutex_t tlock;
    pthread_cond_t tcond;
    int rc;
    struct timespec   ts;
    struct timeval    tp;
        
    pthread_mutex_init(&tlock, NULL);
    pthread_cond_init(&tcond, NULL);

    while(true) {
        send(sock, ack_x0, sizeof(ack_x0), 0);
        send(sock, ack_x2, sizeof(ack_x2), 0);

        rc =  gettimeofday(&tp, NULL);

        /* Convert from timeval to timespec */
        ts.tv_sec  = tp.tv_sec;
        ts.tv_nsec = tp.tv_usec * 1000;
        ts.tv_sec += SLEEP_TIME;

        pthread_cond_timedwait(&tcond, &tlock, &ts);
    }
}

void add_connection(connect_inst_s *c)
{
    if(connlist)
        connlist_tail->next = c;
    else
        connlist = c;
    connlist_tail = c;
}

void wait_message(void)
{
    pthread_mutex_lock(&monitor.lock);
    pthread_cond_wait(&monitor.cond, &monitor.lock);
}

void release_message(void)
{
    pthread_mutex_unlock(&monitor.lock);
}

inline int netgetchar(connect_inst_s *s)
{
redo:
    if(s->i == s->len) {
        s->len = recv(s->sock, s->buf, sizeof(s->buf), 0);
        s->i = 0;
    }
    if(!s->buf[s->i]) {
        s->i++;
        goto redo;
    }

    putchar(s->buf[s->i]);
    //printf("%c~%u   ", s->buf[s->i]);

    fflush(stdout);
    return s->buf[s->i++];
}

/*
 * <uname> := U<id>#*[a-zA-Z_.,]+;[\d]+;[\d]+;[\d]+;[\d]+[\d]+;0
 * <id> := \d\d\d
 * <conn> := C<id>
 * <gamelist> := 0; <gamelist'>
 * <gamelist'> := <gamename>; <gamelist'> | E
 * <gamename> := [a-zA-Z]+
 */

inline char *nexttoken(connect_inst_s *s)
{
    int c, i = 0, diff;
    token_s *t = &s->tok;
    char *lex = t->lexeme;

#define is_idchar_() ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z'))

    c = netgetchar(s);
    switch(c) {
        case '0':
            c = netgetchar(s);
            if(c == '1') {
                c = netgetchar(s);
                if(c == '_') {
                    //known to be 1st received item

                }
            }
            else if(c == ';') {
                c = netgetchar(s);
                while(is_gamenamechar(c)) {
                    *lex++ = c;
                    c = netgetchar(s);
                }
                if(lex - t->lexeme > 0)
                    *--lex = '\0';
                else
                    *lex = '\0';

                printf("\n\ngame name: %s\n\n", t->lexeme);
            }
            break;
        case 'C':
            c = netgetchar(s);
            if(is_idchar_()) {
                c = netgetchar(s);
                if(is_idchar_()){
                    c = netgetchar(s);
                    if(is_idchar_()) {
                        //got C<id> item....
                    }
                }
            }
            break;
        case 'U':
            c = netgetchar(s);
            if(is_idchar_()) {
                c = netgetchar(s);
                if(is_idchar_()){
                    c = netgetchar(s);
                    if(is_idchar_()) {
                        //got C<id> item....
                    }
                }
            }
            do{
                c = netgetchar(s);
                i++;
            } while(c == '#');
            diff = MAX_UNAME_PASS-(i-1);
            break;

    }
exit_:
    t->lexeme[i] = '\0';
    return t;
#undef is_idchar_
}

inline bool is_namechar(int c)
{
    return  (c >= 'a' && c <= 'z')
            ||
            (c >= '1' && c <= '9')
            ||
            (c == '.' || c == '_' || c == ',')
            ||
            (c >= 'A' && c <= 'Z');
}

inline bool is_gamenamechar(int c)
{
    return  (c >= 'a' && c <= 'z')
            ||
            (c >= '0' && c <= '9')
            ||
            c == '.' || c == ',' || c == ' ';
}


void send_message(char *message, int sock)
{
    char buf[256];
    
    if(strlen(message) > 254) {
        printf("message too long");
        return;
    }
    buf[0] = '9';
    strcpy(&buf[1], message);
    send(sock, buf, strlen(buf)+1, 0);
}

connect_inst_s *get_connectinst(char *uname)
{
    connect_inst_s *c;
    
    for(c = connlist; c; c = c->next) {
        
    }
    return NULL;
}
