#ifndef CRYPT_H_
#define CRYPT_H_

#include <stdint.h>

typedef struct sha512_s sha512_s;

struct sha512_s
{
    uint64_t word[8];
};

extern void sha512(char *message, sha512_s *digest);


#endif