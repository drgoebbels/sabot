#ifndef SERVERS_H_
#define SERVERS_H_

#ifdef __cplusplus
extern "C" {
#endif


typedef struct server_rec_s server_rec_s;

struct server_rec_s
{
    char *name;
    char *ip;
};

extern inline server_rec_s *server_lookup(char *key);

#ifdef __cplusplus
}
#endif
  

#endif