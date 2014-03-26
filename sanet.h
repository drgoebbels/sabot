#ifndef SANET_H_
#define SANET_H_

#if defined __cplusplus
extern "C" {
#endif

#include <pthread.h>

#define PORT                1138

#define S_REGISTER          "67.19.145.10"

#define S_2D_CENTRAL        "74.86.43.9"
#define S_PAPER_THIN        "74.86.43.8"
#define S_FINE_LINE         "67.19.138.234" //(?)
#define S_U_OF_SA           "67.19.138.236"
#define S_FLAT_WORLD        "74.86.3.220"
#define S_PLANAR_OUTPOST    "67.19.138.235"
#define S_MOBIUS_METROPOLIS "74.86.3.221"
#define S_AMSTERDAM         "94.75.214.10"
#define S_COMPATABILITY     "74.86.3.222"
#define S_QUICKSTART        "67.19.138.236"
#define S_SS_LINEAGE        "74.86.43.10"

#define S_IN_USE            S_2D_CENTRAL

typedef struct chatbox_s chatbox_s;
typedef struct connect_inst_s connect_inst_s;
typedef struct chat_packet_s chat_packet_s;
typedef struct monitor_s monitor_s;

struct chatbox_s
{
    pthread_mutex_t lock;
    chat_packet_s *head;
    chat_packet_s *tail;
};

struct connect_inst_s
{
    char *server;
    char *uname;
    int sock;
    chatbox_s chat;
    pthread_t thread;
    connect_inst_s *next;
};

struct chat_packet_s
{
    size_t size;
    char *data;
    chat_packet_s *next;
    chat_packet_s *prev;
};

struct monitor_s
{
    uint8_t inuse;
    pthread_t thread;
    pthread_cond_t cond;
    pthread_mutex_t lock;
};

extern monitor_s monitor;

extern connect_inst_s *connlist;

extern connect_inst_s *login(char *server, char *uname, char *pass);
extern void wait_message(void);
extern void release_message(void);

#if defined __cplusplus
}
#endif


#endif
