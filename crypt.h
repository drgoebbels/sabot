#ifndef CRYPT_H_
#define CRYPT_H_

#include <stdint.h>

#include "general.h"

typedef struct sha512_s sha512_s;
typedef union salt_s salt_s;

struct sha512_s
{
    uint64_t word[8];
};

union salt_s
{
    uint64_t whole;
    uint16_t quad[4];
    uint8_t oct[8];
};

/*
 Computes a sha512 digest on input message of size len. Function 
 assumes that the memory pointed to by digest is properly allocated
 before the function call. This is the user's responsibility.
 */
extern void sha512(void *message, size_t len, sha512_s *digest);

extern int sha512_equal(sha512_s *d1, sha512_s *d2);

extern void print_digest(sha512_s *digest);

extern salt_s get_salt(void);

#endif