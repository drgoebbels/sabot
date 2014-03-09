#include <stdio.h>

#include "crypt.h"

typedef struct block_s block_s;

struct block_s
{
    union {
        uint8_t b[128];
        uint64_t word[16];
    };
};

static sha512_s H0 = {
    .word = {
    }
};

static inline void zero_block(block_s *block);

static inline uint64_t Ch(uint64_t x, uint64_t y, uint64_t z);
static inline uint64_t Maj(uint64_t x, uint64_t y, uint64_t z);
static inline uint64_t ROTRn(uint64_t x, int n);
static inline uint64_t E_512_0(uint64_t x);
static inline uint64_t E_512_1(uint64_t x);
static inline uint64_t s_512_0(uint64_t x);
static inline uint64_t s_512_1(uint64_t x);

void sha512(void *message, size_t len, sha512_s *digest)
{
    unsigned i, j;
    uint8_t *msg = message;
    size_t l = len*8, div, k, nblocks, r;
    block_s *states, *stptr;
    uint64_t schedule[80];
    uint64_t a, b, c, d, e, f, g, h;
    
    r = (len + 1) % 128;
    nblocks = (len + 1) / 128 + r;
    
    states = allocz(nblocks);
    
    div = (l / 1024) ? l % 1024 : l;
    
    digest->word[0] = 0x6a09e667f3bcc908lu;
    digest->word[1] = 0xbb67ae8584caa73blu;
    digest->word[2] = 0x3c6ef372fe94f82blu;
    digest->word[3] = 0xa54ff53a5f1d36f1lu;
    digest->word[4] = 0x510e527fade682d1lu;
    digest->word[5] = 0x9b05688c2b3e6c1flu;
    digest->word[6] = 0x1f83d9abfb41bd6blu;
    digest->word[7] = 0x5be0cd19137e2179lu;
    
    /* Solve for k such that l+1+k is congruent to 869 mod 1024 */
    k = (896-(div+1)) % 1024;
    
    for(i = 0, stptr = states; i < len; i++) {
        if((i % 128) && i != 0)
            stptr++;
        stptr->b[i] = msg[i];
    }
    if((i % 128) && i != 0)
        stptr++;
    stptr->b[i] |= 0x0001;
    stptr->word[15] = l;
    
    stptr = states;
    for(i = 0; i < nblocks; i++) {
        for(j = 0; j < 16; j++)
            schedule[j] = stptr->word[j];
        while(j < 80) {
            schedule[j] = s_512_1(schedule[j-2]) + schedule[j-7] + s_512_0(schedule[j-15]) + schedule[j-16];
            j++;
        }
        stptr++;
    }
}

inline void zero_block(block_s *block)
{
    unsigned i;
    
    for(i = 0; i < 16; i++)
        block->word[i] = 0lu;
}

inline uint64_t Ch(uint64_t x, uint64_t y, uint64_t z)
{
    return (x & y) ^ (~x & z);
}

inline uint64_t Maj(uint64_t x, uint64_t y, uint64_t z)
{
    return (x & y) ^ (x & z) ^ (y & z);
}

inline uint64_t ROTRn(uint64_t x, int n)
{
    return (x >> n) | (x << (64 - n));
}

inline uint64_t E_512_0(uint64_t x)
{
    return ROTRn(x, 28) ^ ROTRn(x, 34) ^ ROTRn(x, 39);
}

inline uint64_t E_512_1(uint64_t x)
{
    return ROTRn(x, 14) ^ ROTRn(x, 18) ^ ROTRn(x, 41);
}

inline uint64_t s_512_0(uint64_t x)
{
    return ROTRn(x, 1) ^ ROTRn(x, 8) ^ (x >> 7);
}

inline uint64_t s_512_1(uint64_t x)
{
    return ROTRn(x, 19) ^ ROTRn(x, 61) ^ (x >> 6);
}

