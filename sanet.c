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

#define SLEEP_TIME  10
#define BUF_SIZE 1024
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
static void send_thread(connect_inst_s *c);
static void add_connection(connect_inst_s *c);

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
    pthread_mutex_init(&conn->chat.lock, NULL);
        
    add_connection(conn);
    
    pthread_create(&conn->thread, NULL, (void *(*)(void *))connect_thread, conn);
    pthread_create(&conn->thread, NULL, (void *(*)(void *))send_thread, conn);

    return conn;
}

void connect_thread(connect_inst_s *c)
{
    uint64_t count = 0;
    ssize_t len, i;
    int sock = c->sock;
    char buf[BUF_SIZE];
    chat_packet_s *packet;    
    

    while(1) {

       len = recv(sock, buf, sizeof(buf), 0);
            for(i = 0; i < len; i++)
                putchar(buf[i]);
            putchar('\n');

        packet = alloc(sizeof(*packet));
        packet->data = alloc(len);
        memcpy(packet->data, buf, len);

        if(monitor.inuse) {
           // printf("Thread Started! %lu\n", len);
           // fflush(stdout);
            pthread_mutex_lock(&monitor.lock);
         //   printf("Got Lock!\n");
          //  fflush(stdout);
            pthread_cond_signal(&monitor.cond);
            pthread_mutex_unlock(&monitor.lock);
        }
        count++;
    }
}

void send_thread(connect_inst_s *c)
{
    int sock = c->sock;
    pthread_mutex_t tlock;
    pthread_cond_t tcond;
    int rc;
    struct timespec   ts;
    struct timeval    tp;
    
    
    rc =  gettimeofday(&tp, NULL);

    
    /* Convert from timeval to timespec */
    ts.tv_sec  = tp.tv_sec;
    ts.tv_nsec = tp.tv_usec * 1000;
    ts.tv_sec += SLEEP_TIME;
    
    pthread_mutex_init(&tlock, NULL);
    pthread_cond_init(&tcond, NULL);

    
    while(true) {
        send(sock, ack_x0, sizeof(ack_x0), 0);
        send(sock, ack_x1, sizeof(ack_x1), 0);
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
    puts("Woke Up~~~");
    fflush(stdout);
}

void release_message(void)
{
    pthread_mutex_unlock(&monitor.lock);
}
