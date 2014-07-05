#ifndef REGEX_H_
#define REGEX_H_

#include <stdbool.h>

typedef struct regx_val_s regx_val_s;

typedef struct regex_s regex_s;
typedef struct fsmedge_s fsmedge_s;
typedef struct fsmnode_s fsmnode_s;
typedef struct regerr_s regerr_s;
typedef struct nfa_s nfa_s;
typedef struct repet_s repet_s;

struct regx_val_s {
    bool is_scalar;
    union {
        char c;
        struct {
            char low;
            char high;
        };
    };
};

struct regex_s
{
    bool isnfa;
    regerr_s *err;
    nfa_s *nfa;
};

struct fsmedge_s
{
    regx_val_s val;
    fsmnode_s *child;
    fsmnode_s *parent;
};

struct fsmnode_s
{
    unsigned nedges;
    unsigned blocksize;
    repet_s *rep;
    fsmedge_s **edges;
};

struct regerr_s
{
    regerr_s *next;
    unsigned short pos;
    char msg[];
};

struct nfa_s
{
    fsmnode_s *start;
    fsmnode_s *final;
};

struct repet_s
{
    unsigned low;
    unsigned high;
    unsigned count;
};

extern regex_s *compile_regex(const char *src);

#endif
