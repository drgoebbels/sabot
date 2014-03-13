/*
 Reinventing the wheel for lulz
 */
#include <stdio.h>
#include <string.h>

#include "crypt.h"

typedef struct block_s block_s;

struct block_s
{
    union {
        uint8_t b[128];
        uint64_t word[16];
    };
};

static void zero_block(block_s * volatile block);

static inline uint64_t Ch(uint64_t x, uint64_t y, uint64_t z);
static inline uint64_t Maj(uint64_t x, uint64_t y, uint64_t z);
static inline uint64_t ROTRn(uint64_t x, unsigned n);
static inline uint64_t E_512_0(uint64_t x);
static inline uint64_t E_512_1(uint64_t x);
static inline uint64_t s_512_0(uint64_t x);
static inline uint64_t s_512_1(uint64_t x);
static inline uint64_t to_big_endian(uint64_t w);
static void print_word(uint64_t w);

/*
 
 abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu
 */

#define INNER_ROUND()   T1 = h + E_512_1(e) + Ch(e, f, g) + K[t] + W[t];    \
                        T2 = E_512_0(a) + Maj(a, b, c); \
                        h = g;  g = f;  \
                        f = e;  e = d + T1; \
                        d = c;  c = b;  \
                        b = a;  a = T1 + T2;    \
                        t++

#define ZERO_WORD()     ((uint64_t volatile *)W)[t] = 0llu; \
                        t++



void sha512(void *message, size_t len, sha512_s *digest)
{
    unsigned i, t;
    uint8_t *msg = message;
    size_t l = len*8, div, k, nblocks;
    block_s *states, *stptr;
    uint64_t W[80];
    uint64_t *H = digest->word;
    uint64_t a, b, c, d, e, f, g, h, T1, T2;
    static const uint64_t K[80] = {
        0x428a2f98d728ae22llu, 0x7137449123ef65cdllu, 0xb5c0fbcfec4d3b2fllu, 0xe9b5dba58189dbbcllu,
        0x3956c25bf348b538llu, 0x59f111f1b605d019llu, 0x923f82a4af194f9bllu, 0xab1c5ed5da6d8118llu,
        0xd807aa98a3030242llu, 0x12835b0145706fbellu, 0x243185be4ee4b28cllu, 0x550c7dc3d5ffb4e2llu,
        0x72be5d74f27b896fllu, 0x80deb1fe3b1696b1llu, 0x9bdc06a725c71235llu, 0xc19bf174cf692694llu,
        0xe49b69c19ef14ad2llu, 0xefbe4786384f25e3llu, 0x0fc19dc68b8cd5b5llu, 0x240ca1cc77ac9c65llu,
        0x2de92c6f592b0275llu, 0x4a7484aa6ea6e483llu, 0x5cb0a9dcbd41fbd4llu, 0x76f988da831153b5llu,
        0x983e5152ee66dfabllu, 0xa831c66d2db43210llu, 0xb00327c898fb213fllu, 0xbf597fc7beef0ee4llu,
        0xc6e00bf33da88fc2llu, 0xd5a79147930aa725llu, 0x06ca6351e003826fllu, 0x142929670a0e6e70llu,
        0x27b70a8546d22ffcllu, 0x2e1b21385c26c926llu, 0x4d2c6dfc5ac42aedllu, 0x53380d139d95b3dfllu,
        0x650a73548baf63dellu, 0x766a0abb3c77b2a8llu, 0x81c2c92e47edaee6llu, 0x92722c851482353bllu,
        0xa2bfe8a14cf10364llu, 0xa81a664bbc423001llu, 0xc24b8b70d0f89791llu, 0xc76c51a30654be30llu,
        0xd192e819d6ef5218llu, 0xd69906245565a910llu, 0xf40e35855771202allu, 0x106aa07032bbd1b8llu,
        0x19a4c116b8d2d0c8llu, 0x1e376c085141ab53llu, 0x2748774cdf8eeb99llu, 0x34b0bcb5e19b48a8llu,
        0x391c0cb3c5c95a63llu, 0x4ed8aa4ae3418acbllu, 0x5b9cca4f7763e373llu, 0x682e6ff3d6b2b8a3llu,
        0x748f82ee5defb2fcllu, 0x78a5636f43172f60llu, 0x84c87814a1f0ab72llu, 0x8cc702081a6439ecllu,
        0x90befffa23631e28llu, 0xa4506cebde82bde9llu, 0xbef9a3f7b2c67915llu, 0xc67178f2e372532bllu,
        0xca273eceea26619cllu, 0xd186b8c721c0c207llu, 0xeada7dd6cde0eb1ellu, 0xf57d4f7fee6ed178llu,
        0x06f067aa72176fballu, 0x0a637dc5a2c898a6llu, 0x113f9804bef90daellu, 0x1b710b35131c471bllu,
        0x28db77f523047d84llu, 0x32caab7b40c72493llu, 0x3c9ebe0a15c9bebcllu, 0x431d67c49c100d4cllu,
        0x4cc5d4becb3e42b6llu, 0x597f299cfc657e2allu, 0x5fcb6fab3ad6faecllu, 0x6c44198c4a475817llu
    };
    
    
    H[0] = 0x6a09e667f3bcc908llu;
    H[1] = 0xbb67ae8584caa73bllu;
    H[2] = 0x3c6ef372fe94f82bllu;
    H[3] = 0xa54ff53a5f1d36f1llu;
    H[4] = 0x510e527fade682d1llu;
    H[5] = 0x9b05688c2b3e6c1fllu;
    H[6] = 0x1f83d9abfb41bd6bllu;
    H[7] = 0x5be0cd19137e2179llu;
    
    nblocks = (len + 1) / 128 + !!((len + 1) % 128);
    states = allocz(nblocks * sizeof(*states));
    div = (l / 1024) ? l % 1024 : l;
    
    /* Solve for k such that l+1+k is congruent to 869 mod 1024 */
    k = (896-(div+1)) % 1024;
    
    for(i = 0, stptr = states; i < len; i++) {
        if(!(i % 128) && i != 0)
            stptr++;
        stptr->b[i % 128] = msg[i];
    }
    
    if(!(i % 128) && i != 0)
        stptr++;
    stptr->b[i % 128] |= 0x80;
    
    /* Add size so that its representation is guaranteed to be big endian. */
    stptr->b[127] = l;
    stptr->b[126] = l >> 8;
    stptr->b[125] = l >> 24;
    stptr->b[124] = l >> 32;
    stptr->b[123] = l >> 40;
    stptr->b[122] = l >> 48;
    stptr->b[121] = l >> 56;
    
    stptr = states;
    for(i = 0; i < nblocks; i++, stptr++) {
        for(t = 0; t < 16; t++) {
            W[t] = to_big_endian(stptr->word[t]);
            //printf("W%u=\t", t);
            //print_word(W[t]);
        }
        while(t < 80) {
            W[t] = s_512_1(W[t-2]) + W[t-7] + s_512_0(W[t-15]) + W[t-16];
            t++;
            W[t] = s_512_1(W[t-2]) + W[t-7] + s_512_0(W[t-15]) + W[t-16];
            t++;
            W[t] = s_512_1(W[t-2]) + W[t-7] + s_512_0(W[t-15]) + W[t-16];
            t++;
            W[t] = s_512_1(W[t-2]) + W[t-7] + s_512_0(W[t-15]) + W[t-16];
            t++;
        }
        
        a = H[0];   b = H[1];
        c = H[2];   d = H[3];
        e = H[4];   f = H[5];
        g = H[6];   h = H[7];

        /* Many tests showed some loop unravelling brought performance increase */
        for(t = 0; t < 80; ) {
            INNER_ROUND();
            INNER_ROUND();
            INNER_ROUND();
            INNER_ROUND();
            //printf("t=%u:\t%llx\t%llx\t%llx\t%llx\n\t%llx\t%llx\t%llx\t%llx\n", t, a, b, c, d, e, f, g, h);
        }

        H[0] += a;  H[1] += b;
        H[2] += c;  H[3] += d;
        H[4] += e;  H[5] += f;
        H[6] += g;  H[7] += h;
    }
    stptr = states;
    
    /* Zero out plaintext from memory */
    t = 0;
    while(t < 80) {
        ZERO_WORD();
        ZERO_WORD();
        ZERO_WORD();
        ZERO_WORD();
    }
    for(i = 0; i < nblocks; i++)
        zero_block(&stptr[i]);
    free(states);
}

void zero_block(block_s * volatile block)
{
    unsigned i;
    
    for(i = 0; i < 16; i++)
        block->word[i] = 0llu;
}

inline uint64_t Ch(uint64_t x, uint64_t y, uint64_t z)
{
    return (x & y) ^ (~x & z);
}

inline uint64_t Maj(uint64_t x, uint64_t y, uint64_t z)
{
    return (x & y) ^ (x & z) ^ (y & z);
}

inline uint64_t ROTRn(uint64_t x, unsigned n)
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

inline uint64_t to_big_endian(uint64_t w)
{
    uint64_t res;
    register uint8_t *pres = (uint8_t *)&res;
    
    pres[0] = w >> 56;
    pres[1] = w >> 48;
    pres[2] = w >> 40;
    pres[3] = w >> 32;
    pres[4] = w >> 24;
    pres[5] = w >> 16;
    pres[6] = w >> 8;
    pres[7] = w;
    return res;
}

void print_word(uint64_t w)
{
    unsigned i;
    uint8_t *b = (uint8_t *)&w;
    
    for(i = 0; i < sizeof(w); i++) {
        printf("%02x", b[i]);
    }
    putchar('\n');
}

void print_digest(sha512_s *digest)
{
    unsigned i;
    uint64_t *p = (uint64_t *)digest->word;
    
    for(i = 0; i < 8; i++, p++)
        printf("%llx ", *p);
    
}

