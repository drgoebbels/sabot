#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "general.h"
#include "sanet.h"

#define BUF_SIZE 1024
#define LOGIN_FLAG "09"

struct connect_inst_s
{
    char *server;
    char *uname;
    int sock;
    pthread_t thread;
};

/* Initial Packet Sent when logging in */
static char init_send[] = {
    0x30, 0x38, 0x48, 0x78, 0x4f, 0x39, 0x54, 0x64,
    0x43, 0x43, 0x36, 0x32, 0x4e, 0x77, 0x6c, 0x6e,
    0x31, 0x50, 0x00
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

static int socket_(char *server);
static void connect_thread(connect_inst_s *c);

int socket_(char *server)
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

connect_inst_s *login(char *server, char *uname, char *pass)
{
    int sock;
    char buf[BUF_SIZE];
    connect_inst_s *c;
    
    if(strlen(uname) + strlen(pass) + sizeof(LOGIN_FLAG) + 1 > BUF_SIZE) {
        fprintf(stderr, "Input too large you retard\n");
        return NULL;
    }
    
    c = alloc(sizeof(*c));
    c->server = server;
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
    pthread_create(&c->thread, NULL, (void *(*)(void *))connect_thread, c);
    
    pthread_join(c->thread, NULL);
    
    return c;
}

void connect_thread(connect_inst_s *c)
{
    unsigned i;
    ssize_t len;
    int sock = c->sock;
    char buf[BUF_SIZE];
    
    while(1) {
        send(sock, ack_x1, sizeof(ack_x2), 0);
        len = recv(sock, buf, sizeof(buf), 0);
        
        for(i = 0; i < len; i++)
            putchar(buf[i]);
        putchar('\n');
    }
}
