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
    unsigned i, j, t;
    uint8_t *msg = message;
    size_t l = len*8, div, k, nblocks, r;
    block_s *states, *stptr;
    uint64_t W[80];
    uint64_t *H = digest->word;
    uint64_t a, b, c, d, e, f, g, h, T1, T2;
    
    static uint64_t K[80] = {
        0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc,
        0x3956c25bf348b538, 0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118,
        0xd807aa98a3030242, 0x12835b0145706fbe, 0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2,
        0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235, 0xc19bf174cf692694,
        0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65,
        0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5,
        0x983e5152ee66dfab, 0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4,
        0xc6e00bf33da88fc2, 0xd5a79147930aa725, 0x06ca6351e003826f, 0x142929670a0e6e70,
        0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed, 0x53380d139d95b3df,
        0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b,
        0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30,
        0xd192e819d6ef5218, 0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8,
        0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8,
        0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373, 0x682e6ff3d6b2b8a3,
        0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec,
        0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b,
        0xca273eceea26619c, 0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178,
        0x06f067aa72176fba, 0x0a637dc5a2c898a6, 0x113f9804bef90dae, 0x1b710b35131c471b,
        0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc, 0x431d67c49c100d4c,
        0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817
    };
    
    r = (len + 1) % 128;
    nblocks = (len + 1) / 128 + r;
    
    states = allocz(nblocks);
    
    div = (l / 1024) ? l % 1024 : l;
    
    H[0] = 0x6a09e667f3bcc908lu;
    H[1] = 0xbb67ae8584caa73blu;
    H[2] = 0x3c6ef372fe94f82blu;
    H[3] = 0xa54ff53a5f1d36f1lu;
    H[4] = 0x510e527fade682d1lu;
    H[5] = 0x9b05688c2b3e6c1flu;
    H[6] = 0x1f83d9abfb41bd6blu;
    H[7] = 0x5be0cd19137e2179lu;
    
    /* Solve for k such that l+1+k is congruent to 869 mod 1024 */
    k = (896-(div+1)) % 1024;
    
    for(i = 0, stptr = states; i < len; i++, stptr++) {
        if((i % 128) && i != 0)
            stptr++;
        stptr->b[i] = msg[i];
    }
    if((i % 128) && i != 0)
        stptr++;
    stptr->b[i] |= 0x0001;
    stptr->word[15] = l;
    
    stptr = states;
    for(t = 1; t < nblocks; t++) {
        for(j = 0; j < 16; j++)
            W[j] = stptr->word[j];
        while(j < 80) {
            W[j] = s_512_1(W[j-2]) + W[j-7] + s_512_0(W[j-15]) + W[j-16];
            j++;
        }
        
        a = H[0];
        b = H[1];
        c = H[2];
        d = H[3];
        e = H[4];
        f = H[5];
        g = H[6];
        h = H[7];
        
        T1 = h + E_512_1(e) + Ch(e, f, g) + K[t] + W[t];
        T2 = E_512_0(a) + Maj(a, b, c);
        h = g;
        g = f;
        f = e;
        e = T1;
        d = c;
        c = b;
        b = a;
        a = T1 + T2;
        
        H[0] += a;
        H[1] += b;
        H[2] += c;
        H[3] += d;
        H[4] += e;
        H[5] += f;
        H[6] += g;
        H[7] += h;
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

void print_digest(sha512_s *digest)
{
    unsigned i;
    uint8_t *p = (uint8_t *)digest->word;
    
    for(i = 0; i < 128; i++, p++)
        printf("%x", *p);
    
}

