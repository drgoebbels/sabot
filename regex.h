#ifndef REGEX_H_
#define REGEX_H_

#include <stdbool.h>

typedef struct regex_s regex_s;
typedef struct fsmedge_s fsmedge_s;
typedef struct fsmnode_s fsmnode_s;

struct regex_s
{
    bool isnfa;
    fsmnode_s *start;
};

struct fsmedge_s
{
    bool isclass;
    union {
        char c;
        fsmedge_s *set;
    };
};

struct fsmnode_s
{
    unsigned nedges;
    fsmedge_s edges[];
};

extern regex_s *compile_regex(const char *src);

#endif
