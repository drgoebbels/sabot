#ifndef SANET_H_
#define SANET_H_

#if defined __cplusplus
extern "C" {
#endif

#include <sys/time.h>
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
#define BUF_SIZE 1024
#define LEXEME_SIZE 128
#define MAX_UNAME_PASS 20
#define UID_TABLE_SIZE 53

    
enum event_types_e {
    EVENT_CHAT_MSG,
    EVENT_EDIT_USERS,
    EVENT_EDIT_GAMES
};

typedef struct chatbox_s chatbox_s;
typedef struct token_s token_s;

typedef struct chat_event_s chat_event_s;
typedef struct message_s message_s;
typedef struct edit_users_s edit_users_s;
typedef struct edit_games_s edit_games_s;

typedef struct connect_inst_s connect_inst_s;
typedef struct chat_packet_s chat_packet_s;
typedef struct monitor_s monitor_s;
typedef struct user_s user_s;
    
typedef struct uid_record_s uid_record_s;
typedef struct uid_hash_s uid_hash_s;
typedef struct gamelist_s gamelist_s;


struct chatbox_s
{
    pthread_mutex_t lock;
    chat_packet_s *head;
    chat_packet_s *tail;
};
    
struct token_s
{
    int type;
    char lexeme[LEXEME_SIZE];
    token_s *next;
};

struct chat_event_s
{
    chat_event_s *prev;
    chat_event_s *next;
    user_s *user;
    time_t timestamp;
    enum event_types_e type;
};

struct message_s
{
    chat_event_s base;
    char text[148];
    char type;
};

struct edit_users_s
{
    chat_event_s base;
    bool add;
};
    
struct edit_games_s
{
    chat_event_s base;
    gamelist_s *glist;
    gamelist_s *tail;
};

struct connect_inst_s
{
    const char *server;
    const char *uname;
    int sock;
    chatbox_s chat;
    pthread_t thread;
    size_t len;
    size_t i;
    char buf[BUF_SIZE];
    token_s tok;
    struct {
        chat_event_s *head;
        chat_event_s *tail;
    }
    uqueue;
    connect_inst_s *next;
};

struct chat_packet_s
{
    bool is_consumed;
    user_s *user;
    char type;
    char text[148];
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

struct user_s
{
    char id[4];
    char name[MAX_UNAME_PASS+1];
    /* These are Still unknown data fields */
    char field1[32];
    char field2[32];
    char field3[32];

    char field4[32];
    char field5[32];
    char field6[32];
    char mod_level;
};
    
struct uid_record_s
{
    user_s *user;
    uid_record_s *next;
};

struct uid_hash_s
{
    int size;
    uid_record_s *table[UID_TABLE_SIZE];
};
    
struct gamelist_s
{
    char name[32];
    gamelist_s *next;
};
    
extern uid_hash_s sanet_users;
    
extern monitor_s monitor;

extern connect_inst_s *connlist;
extern connect_inst_s *conncurr;

extern connect_inst_s *login(const char *server, const char *uname, const char *pass);
extern void send_message(connect_inst_s *conn, const char *message, const char *prefix);
extern void send_pmessage(connect_inst_s *conn, const char *message, const char *id);

extern void wait_message(void);
extern void release_message(void);
extern connect_inst_s *get_connectinst(char *uname);
extern inline void msg_lock(connect_inst_s *conn);
extern inline void msg_unlock(connect_inst_s *conn);

extern void event_enqueue(connect_inst_s *conn, chat_event_s *event);
extern chat_event_s *event_dequeue(connect_inst_s *conn);

extern void adduser(user_s *u);
extern user_s *userlookup(char *uid);
extern void deleteuser(char *uid);

#if defined __cplusplus
}
#endif


#endif
