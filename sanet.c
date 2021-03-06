#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <assert.h>

#include "general.h"
#include "sanet.h"
#include "database.h"

#ifdef NDEBUG
    #define	traffic_log(a) ((void)0)
#endif

//#define DEBUG_SIMULA

#define SLEEP_TIME 5
#define LOGIN_FLAG "09"

uid_hash_s sanet_users;

connect_inst_s *connlist;
connect_inst_s *conncurr;

monitor_s monitor = {
    .inuse = 0,
    .cond = PTHREAD_COND_INITIALIZER,
    .lock = PTHREAD_MUTEX_INITIALIZER
};

static FILE *traf_log, *stats;
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


static user_s err_user = {
    .name = ERROR_USER_NAME
};


#define N_INSULTS 4
static const char *insults[] = {
    "is the prince of turds.",
    "likes to dress up his poopies in uniform.",
    "is a disgrace to his own acne.",
    "is physically attracted to bald chipmunks."
};

static bool check_login(const char *server, const char *uname, const char *pass);
static int connect_(const char *server);
static int login_(connect_inst_s *conn);
static void connect_thread(connect_inst_s *c);
static void ack_thread(connect_inst_s *c);
static void add_connection(connect_inst_s *c);

static int netgetc(connect_inst_s *conn);
static inline char *nexttoken(connect_inst_s *conn);
static inline bool is_namechar(int c);
static inline bool is_gamenamechar(int c);
static void glist_add(edit_games_s *games, gamelist_s *node);

static user_s *parse_uname(connect_inst_s *conn);
static void print_user(user_s *s);
static void printstr_n(char *str, size_t size);
static inline void traffic_log(int c);
static bool isevil_name(char *username);
static void check_insult(message_s *msg, connect_inst_s *c);

static bool spam_check(message_s *msg);

static uint16_t uid_hash(char *uid);

void sanet_init(void)
{
    /*
    sa.sa_handler = sigUSR1;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    status = sigaction(SIGUSR1, &sa, NULL);
    if(status < 0) {
        perror("Error installing handler for SIGUSR1");
        exit(EXIT_FAILURE);
    }*/
}

bool check_login(const char *server, const char *uname, const char *pass)
{
    return false;
}

int connect_(const char *server)
{
    int sock, res;
    struct sockaddr_in sockin;
    
#ifdef DEBUG_SIMULA
    traf_log = fopen(server, "r");
    if(!traf_log) {
        perror("Error Opening File");
        exit(EXIT_FAILURE);
    }
    return fileno(traf_log);
#else
    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < -1) {
        perror("cannot create socket");
        exit(EXIT_FAILURE);
    }
    
    memset(&sockin, 0, sizeof(sockin));

    puts("code:");
    int i = 0;
    for(; i < sizeof(init_send); i++)
        putchar(init_send[i]);

    putchar('\n');
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
        sock = -1;
    }
    return sock;
#endif
}

connect_inst_s *login(const char *server, const char *uname, const char *pass)
{
    connect_inst_s *conn;
    char lname[strlen(uname)+sizeof(".log")];
    
    stats = fopen("stats.log", "w");
    if(!stats) {
        fprintf(stderr, "Unable to create stats file for logging");
        return NULL;
    }

    if(strlen(uname) + strlen(pass) + sizeof(LOGIN_FLAG) + 1 > 2*MAX_UNAME_PASS+5) {
        fprintf(stderr, "Input too large you retard\n");
        return NULL;
    }
    
#ifndef NDEBUG
    strcpy(lname, uname);
    strcpy(&lname[strlen(uname)], ".log");
    traf_log = fopen(lname, "w");
    if(!traf_log) {
        perror("Failed To Create Log File");
        exit(EXIT_FAILURE);
    }
#endif
    

    conn = alloc(sizeof(*conn));
    conn->uname = strdup(uname);
    conn->pass = strdup(pass);
    conn->server = server;
    conn->chat.head = NULL;
    conn->chat.tail = NULL;
    conn->next = NULL;
    pthread_mutex_init(&conn->chat.lock, NULL);
    pthread_cond_init(&conn->ack_timer, NULL);

    login_(conn);
    
    return conn;
}

int login_(connect_inst_s *conn)
{
    int c;
    user_s *self;
    edit_users_s *edit;
    time_t timestamp;
    char buf[2*MAX_UNAME_PASS+5];

    conn->i = 0;
    conn->len = 0;
    conn->isactive = true;
    conn->isacking = true;
    conn->uqueue.head = NULL;
    conn->sock = connect_(conn->server);

    if(conn->sock < 0)
        return -1;

    send(conn->sock, init_send, sizeof(init_send), 0);
    recv(conn->sock, buf, sizeof(buf), 0);

    sprintf(buf, LOGIN_FLAG "%s;%s", conn->uname, conn->pass);
    send(conn->sock, buf, strlen(buf)+1, 0);

    c = netgetc(conn);
    if(c == 'A') {
        self = parse_uname(conn);
        adduser(self);

        timestamp = time(NULL);
        dbadd_user_record(self, conn->server, timestamp);

        edit = alloc(sizeof(*edit));
        edit->base.type = EVENT_EDIT_USERS;
        edit->base.user = self;
        edit->base.timestamp = timestamp;
        edit->add = true;

        pthread_mutex_lock(&conn->chat.lock);
        event_enqueue(conn, (chat_event_s *)edit);
        pthread_mutex_unlock(&conn->chat.lock);

        pthread_mutex_lock(&monitor.lock);
        pthread_cond_signal(&monitor.cond);
        pthread_mutex_unlock(&monitor.lock);
        netgetc(conn);
    }

    send(conn->sock, ack_x1, sizeof(ack_x1), 0);
    send(conn->sock, ack_x2, sizeof(ack_x2), 0);
    send(conn->sock, finish_login, sizeof(finish_login), 0);

    pthread_mutex_lock(&monitor.lock);
    add_connection(conn);
    pthread_mutex_unlock(&monitor.lock);

    conncurr = conn;

    pthread_create(&conn->conn_thread, NULL, (void *(*)(void *))connect_thread, conn);
    pthread_create(&conn->ack_thread, NULL, (void *(*)(void *))ack_thread, conn);
}

int change_server(connect_inst_s *conn, const char *server)
{

    conn->isactive = false;

    pthread_cond_signal(&conn->ack_timer);

    pthread_join(conn->conn_thread, NULL);
    conn->isacking = false;
    pthread_cond_signal(&conn->ack_timer);

    pthread_join(conn->ack_thread, NULL);

    puts("called change server\n");
    fflush(stdout);

    conn->server = server;

    return login_(conn);
}

connect_inst_s *quickstart(const char *server)
{
    //send(sock, init_send, sizeof(init_send), 0);
    //recv(sock, buf, sizeof(buf), 0);

}

/*
 * <uname> := U<id>#*[a-zA-Z_.,]+;[\d]+;[\d]+;[\d]+;[\d]+[\d]+;0
 * <id> := \d\d\d
 * <conn> := C<id>
 * <gamelist> := 0; <gamelist'>
 * <gamelist'> := <gamename>; <gamelist'> | E
 * <gamename> := [a-zA-Z]+
 */

#define is_idchar_() ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z'))

void connect_thread(connect_inst_s *conn)
{
    user_s *u;
    int c, i = 0;
    chatbox_s *chptr;
    time_t timestamp;
    char *lex;
    gamelist_s *node;
    char lexbuf[32];
    union {
        chat_event_s *event;
        message_s *message;
        edit_users_s *edit;
        edit_games_s *game;
    } events;
       
    while(conn->isactive) {
        i = 0;
        lex = lexbuf;
        c = netgetc(conn);

        timestamp = time(NULL);

        switch(c) {
            case '0':
                c = netgetc(conn);
                if(c == '1') {
                    c = netgetc(conn);
                    if(c == '_') {
                        while(!(c = netgetc(conn)));
                        if(c == '0' && netgetc(conn) == ';') {
                            c = netgetc(conn);
                            events.game = alloc(sizeof(*events.game));
                            events.game->glist = NULL;
                            while(c) {
                                node = alloc(sizeof(*node));
                                lex = node->name;
                                *lex++ = c;
                                while((c = netgetc(conn)) != ';')
                                    *lex++ = c;
                                if(lex - node->name > 1) {
                                    *--lex = '\0';
                                    node->next = NULL;
                                    glist_add(events.game, node);
                                }
                                else {
                                    free(node);
                                }
                                c = netgetc(conn);
                            }
                            events.game->base.timestamp = timestamp;
                            events.game->base.user = NULL;
                            events.game->base.type = EVENT_EDIT_GAMES;

                            /* game list to event queue */
                            pthread_mutex_lock(&conn->chat.lock);
                            event_enqueue(conn, events.event);
                            pthread_mutex_unlock(&conn->chat.lock);

                            /* signal monitor thread to update game list in Qt */
                            pthread_mutex_lock(&monitor.lock);
                            pthread_cond_signal(&monitor.cond);
                            pthread_mutex_unlock(&monitor.lock);
                        }
                    }
                }
                break;
            case 'C':
                c = netgetc(conn);
                if(is_idchar_()) {
                    c = netgetc(conn);
                    if(is_idchar_()){
                        c = netgetc(conn);
                    }
                }
                netgetc(conn);
                break;
            case 'U':
                u = parse_uname(conn);
                adduser(u);
                
                dbadd_user_record(u, conn->server, timestamp);

                events.edit = alloc(sizeof(*events.edit));
                events.edit->base.type = EVENT_EDIT_USERS;
                events.edit->base.user = u;
                events.edit->base.timestamp = timestamp;
                events.edit->add = true;
                
                pthread_mutex_lock(&conn->chat.lock);
                event_enqueue(conn, events.event);
                pthread_mutex_unlock(&conn->chat.lock);
                
                pthread_mutex_lock(&monitor.lock);
                pthread_cond_signal(&monitor.cond);
                pthread_mutex_unlock(&monitor.lock);
                netgetc(conn);
                break;
            case 'M':
                events.message = alloc(sizeof(*events.message));
                
                /* get id of sender */
                *lex++ = netgetc(conn);
                *lex++ = netgetc(conn);
                *lex++ = netgetc(conn);
                *lex = '\0';
                
                u = userlookup(lexbuf);
                
                events.message->base.user = u;
                events.message->base.type = EVENT_CHAT_MSG;
                events.message->base.timestamp = timestamp;
                /* get type of message */
                events.message->type = netgetc(conn);
                
                if(events.message->type == 'P') {
                    //send_pmessage(conn, "I'M AFK YOU SUGAR PLUM TURD FROSTING", lexbuf);
                }

                /* get message content */
                lex = events.message->text;
                while((*lex++ = netgetc(conn)));
                events.message->len = (lex - 1) - events.message->text;
                dblog_message(events.message);

                chptr = &conn->chat;
                
                /* don't spam moderators */
                if((spam_check(events.message) || isevil_name(events.message->base.user->name))) {
                    fprintf(stderr, "SPAM DETECTED!\n");
                    /* Penalize the spammer! */
                      send_pmessage(conn, events.message->text, lexbuf);
                      break;
                }
             //   send_message(conn, events.message->text, "P");
                
                check_insult(events.message, conn);

                pthread_mutex_lock(&chptr->lock);
                event_enqueue(conn, events.event);
                pthread_mutex_unlock(&chptr->lock);

                pthread_mutex_lock(&monitor.lock);
                pthread_cond_signal(&monitor.cond);
                pthread_mutex_unlock(&monitor.lock);
                break;
            case 'D':
                *lex++ = netgetc(conn);
                *lex++ = netgetc(conn);
                *lex++ = netgetc(conn);
                *lex = '\0';
                
                u = userlookup(lexbuf);
                events.edit = alloc(sizeof(*events.edit));
                events.edit->base.user = u;
                events.edit->base.timestamp = timestamp;
                events.edit->base.type = EVENT_EDIT_USERS;
                events.edit->add = false;
                pthread_mutex_lock(&conn->chat.lock);
                event_enqueue(conn, events.event);
                pthread_mutex_unlock(&conn->chat.lock);
                
                pthread_mutex_lock(&monitor.lock);
                pthread_cond_signal(&monitor.cond);
                pthread_mutex_unlock(&monitor.lock);
                deleteuser(lexbuf);
                netgetc(conn);
                break;
            default:
                break;
        }
    }
    puts("exiting conn thread");
    fflush(stdout);
    close(conn->sock);
    pthread_exit(NULL);
}

#undef is_idchar_


void ack_thread(connect_inst_s *c)
{
    uint8_t count = 0;
    int sock = c->sock;
    pthread_mutex_t tlock;
    int rc;
    struct timespec ts;
    struct timeval tp;
    
    pthread_mutex_init(&tlock, NULL);

    while(c->isacking) {
        if(!(count % 32))
            send(sock, ack_x0, sizeof(ack_x0), 0);
        send(sock, ack_x2, sizeof(ack_x2), 0);

        rc = gettimeofday(&tp, NULL);
        
        /* Convert from timeval to timespec */
        ts.tv_sec = tp.tv_sec;
        ts.tv_nsec = tp.tv_usec * 1000;
        ts.tv_sec += SLEEP_TIME;

        pthread_cond_timedwait(&c->ack_timer, &tlock, &ts);
        count++;
    }
    pthread_mutex_unlock(&tlock);
    pthread_mutex_destroy(&tlock);

    puts("exiting ack thread");
    fflush(stdout);

    pthread_exit(NULL);
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

int netgetc(connect_inst_s *s)
{
    if(s->i == s->len) {
        s->len = read(s->sock, s->buf, sizeof(s->buf));
        s->i = 0;
        if(s->len <= 0) {
            puts("Hit EOF or Lost Connection");
            return EOF;
        }
    }
    if(s->buf[s->i])
        putchar(s->buf[s->i]);
    else
        puts("\n");
    traffic_log(s->buf[s->i]);

    if(s->buf[s->i] < 0) {
        perror("Connection Error!");
        exit(EXIT_FAILURE);
    }

    fflush(stdout);
    return s->buf[s->i++];
}

bool spam_check(message_s *msg)
{
    bool ret = false;
    double scale, val;
    size_t len1, len2;
    time_t dt;
    user_s *u = msg->base.user;
    
    if(u->prev && !IS_MOD(u) && (msg->type == '9' || msg->type == 'P')) {
        dt = time(NULL) - u->prev->base.timestamp;
        val = dt*(0.1 + abs(strcasecmp(msg->text, u->prev->text)));
        

        len1 = strlen(u->prev->text);
        len2 = strlen(msg->text);

        scale = (1.0 - (len1 >= len2 ? len2/(double)len1 : len1/(double)len2));
        
        if(scale < 0.1) {
            /* enforce minimum threshold to prevent catastrophic cancellation */
            scale = 0.1;
        }
        
        scale *= val;
                
        free(u->prev);
        
        fprintf(stats, "For %s:\n%s\n%s\nscale: %f, val is: %f, spamcount: ", u->name, u->prev->text, msg->text, scale/val, scale);


        if(scale <= 10.0) {
            if(u->spamcount >= 5) {
                ret = true;
                fputs("SPAM!\n", stats);
            }
            else {
                u->spamcount++;
            }
        }
        else if(u->spamcount) {
           if(u->spamcount < 5) {
               u->spamcount--;
           }
           else {
               u->spamcount++;
               if(u->spamcount == 10)
                   u->spamcount = 4;
           }
        }
        fprintf(stats, "%u\n\n", u->spamcount);
        fflush(stats);
    }
    u->prev = msg;
    return ret;
}


void print_user(user_s *s)
{
    printf("user: %s %s %s %s %s %s %s %s %c\n",
           s->id,
           s->name,
           s->field1,
           s->field2,
           s->field3,
           s->field4,
           s->field5,
           s->field6,
           s->mod_level
           );
}

void printstr_n(char *str, size_t size)
{
    size_t i;
    
    for(i = 0; i < size; i++)
        putchar(str[i]);
}

inline bool is_namechar(int c)
{
    return (c >= 'a' && c <= 'z')
    ||
    (c >= '0' && c <= '9')
    ||
    (c == '.' || c == '_' || c == ',')
    ||
    (c >= 'A' && c <= 'Z');
}

inline bool is_gamenamechar(int c)
{
    return (c >= 'a' && c <= 'z')
    ||
    (c >= '0' && c <= '9')
    ||
    c == '.' || c == ',' || c == ' ' || c == '!';
}

void glist_add(edit_games_s *games, gamelist_s *node)
{
    if(games->glist)
        games->tail->next = node;
    else
        games->glist = node;
    games->tail = node;
}

user_s *parse_uname(connect_inst_s *conn)
{
    char *lex;
    user_s *u;
    int diff, c, i = 0;
    
  //  timestamp = time(NULL);
    
    u = alloc(sizeof(*u));
    
    lex = u->id;
    *lex++ = netgetc(conn);
    *lex++ = netgetc(conn);
    *lex++ = netgetc(conn);
    *lex = '\0';
    
    while((c = netgetc(conn)) == '#')
        i++;
    diff = MAX_UNAME_PASS-i-1;
    
    lex = u->name;
    *lex++ = c;
    for(i = 0; i < diff; i++)
        *lex++ = netgetc(conn);
    *lex = '\0';
    
    lex = u->field1;
    while((c = netgetc(conn)) != ';')
        *lex++ = c;
    *lex = '\0';
    
    lex = u->field2;
    while((c = netgetc(conn)) != ';')
        *lex++ = c;
    *lex = '\0';
    
    lex = u->field3;
    while((c = netgetc(conn)) != ';')
        *lex++ = c;
    *lex = '\0';
    
    lex = u->field4;
    while((c = netgetc(conn)) != ';')
        *lex++ = c;
    *lex = '\0';
    
    lex = u->field5;
    while((c = netgetc(conn)) != ';')
        *lex++ = c;
    *lex = '\0';
    
    lex = u->field6;
    while((c = netgetc(conn)) != ';')
        *lex++ = c;
    *lex = '\0';
        
    u->mod_level = netgetc(conn);
    
    u->spamcount = 0;
    u->prev = NULL;
    
    u->login = -1;
    u->user = -1;

    return u;
}


inline void msg_lock(connect_inst_s *conn)
{
    pthread_mutex_lock(&conn->chat.lock);
}

inline void msg_unlock(connect_inst_s *conn)
{
    pthread_mutex_unlock(&conn->chat.lock);
}

void event_enqueue(connect_inst_s *conn, chat_event_s *event)
{
    if(conn->uqueue.head) {
        conn->uqueue.tail->next = event;
        event->prev = conn->uqueue.tail;
    }
    else {
        event->prev = NULL;
        conn->uqueue.head = event;
    }
    event->next = NULL;
    conn->uqueue.tail = event;
}

chat_event_s *event_dequeue(connect_inst_s *conn)
{
    chat_event_s *next = conn->uqueue.head;
    if(next) {
        conn->uqueue.head = conn->uqueue.head->next;
        if(conn->uqueue.head)
            conn->uqueue.head->prev = NULL;
    }
    return next;
}

void send_message(connect_inst_s *conn, const char *message, const char *prefix)
{
    size_t mlen = strlen(message),
           plen = strlen(prefix);
    char buf[256];

    if(mlen + plen >= 256)
        perror("message to long");
    else {
        strcpy(buf, prefix);
        strcat(buf, message);
        send(conn->sock, buf, strlen(buf)+1, 0);
    }
}

void send_pmessage(connect_inst_s *conn, const char *message, const char *id)
{
    char buf[256];
    char prefix[7];

    if(strlen(message) > 249) {
        printf("message too long");
        return;
    }
    prefix[0] = '0';
    prefix[1] = '0';
    prefix[2] = id[0];
    prefix[3] = id[1];
    prefix[4] = id[2];
    prefix[5] = 'P';
    prefix[6] = '\0';
    send_message(conn, message, prefix);
}

void pm_broadcast(connect_inst_s *conn, const char *message)
{
    send_message(conn, message, "P");
}

connect_inst_s *get_connectinst(char *uname)
{
    connect_inst_s *c;
    
    for(c = connlist; c; c = c->next) {
        
    }
    return NULL;
}

inline void traffic_log(int c)
{
    fputc(c, traf_log);
}

bool isevil_name(char *username)
{
    int i;
    enum size {TSIZE = 2};

    static const char *table[] = {
        "semen", "orgy", "Melody", "beer"
    };

    if(strlen(username) < TSIZE)
        return true;
    for(i = 0; i < TSIZE; i++) {
        if(!strcmp(username, table[i]))
            return true;
    }
    
    return false;
}

void check_insult(message_s *msg, connect_inst_s *c)
{
    user_s *u;
    char *m = msg->text;
    char buf[512], *b = buf;
    char insult[512];
    
    if(*m == ',') {
        if(!strncmp(m+1, "insult", 6)) {
            m += 7;
            if(*m != ' ')
                return;
            while(*++m == ' ');
            while(*m && ((*b++ = *m++) != ' '));
            *b = '\0';
            
            
            
            u = getuser_from_name(buf);
            
            *b++ = ' ';
            *b = '\0';
            
            if(u) {
                strcpy(insult, buf);
                puts("Sending Insult!");
                fflush(stdout);
                sleep(1);
                send_message(c, strcat(buf, insults[rand() % N_INSULTS]), "9");
            }
        }
    }
}

void adduser(user_s *u)
{
    uint16_t index = uid_hash(u->id);
    uint32_t k = *(uint32_t *)u->id, tmp; /* ,,|,, strict aliasing */
    uid_record_s *rec = sanet_users.table[index], *n;

    n = alloc(sizeof(*n));
    n->user = u;
    n->next = NULL;
    if(rec) {
        tmp = *(uint32_t *)rec->user->id;
        if(k == tmp)
            return;
        while(rec->next) {
            tmp = *(uint32_t *)rec->user->id;
            if(k == tmp)
                return;
            rec = rec->next;
        }
        tmp = *(uint32_t *)rec->user->id;
        if(k == tmp)
            return;
        rec->next = n;
    }
    else {
        sanet_users.table[index] = n;
    }
    sanet_users.size++;
}

user_s *userlookup(char *uid)
{
    uint32_t k = *(uint32_t *)uid, tmp;
    uid_record_s *rec = sanet_users.table[uid_hash(uid)];
    
    while(rec) {
        tmp = *(uint32_t *)rec->user->id;
        if(k == tmp)
            return rec->user;
        rec = rec->next;
    }
    fprintf(stderr, "Error: Tried to look up %s\n\n", uid);
    return &err_user;
}

user_s *getuser_from_name(char *name)
{
    int i;
    uid_record_s *rec;
    
    for(i = 0; i < UID_TABLE_SIZE; i++) {
        for(rec = sanet_users.table[i]; rec; rec = rec->next) {
            if(!strcmp(rec->user->name, name)) {
                return rec->user;
            }
        }
    }
    return NULL;
}


void deleteuser(char *uid)
{
    uint16_t index = uid_hash(uid);
    uint32_t k = *(uint32_t *)uid, tmp;
    uid_record_s *rec = sanet_users.table[index], *last = NULL;
    
    while(rec) {
        tmp = *(uint32_t *)rec->user->id;
        if(k == tmp) {
            if(last) {
                last->next = rec->next;
            }
            else {
                sanet_users.table[index] = rec->next;
            }
            free(rec);
            return;
        }
        last = rec;
        rec = rec->next;
    }
}

/* Unravelled version of hashpjw */
uint16_t uid_hash(char *uid)
{
    uint32_t h = 0, g;
    
    h = (h << 4) + uid[0];
    if((g = h & 0xf0000000)) {
        h ^= (g >> 24);
        h ^= g;
    }
    h = (h << 4) + uid[1];
    if((g = h & 0xf0000000)) {
        h ^= (g >> 24);
        h ^= g;
    }
    h = (h << 4) + uid[2];
    if((g = h & 0xf0000000)) {
        h ^= (g >> 24);
        h ^= g;
    }
    assert(!uid[3]);
    return h % UID_TABLE_SIZE;
}
