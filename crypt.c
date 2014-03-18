/*
 Reinventing the wheel for lulz
 
 sha512 implementation as specified by nist. Not entirely compliant due to lack
 of support of 128-bit message lengths.
 */
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "crypt.h"

typedef struct block_s block_s;

struct block_s
{
    union {
        uint8_t b[128];
        uint64_t word[16];
    };
};

static void zero_block(volatile block_s *block);

static inline uint64_t Ch(uint64_t x, uint64_t y, uint64_t z);
static inline uint64_t Maj(uint64_t x, uint64_t y, uint64_t z);
static inline uint64_t ROTRn(uint64_t x, unsigned n);
static inline uint64_t E_512_0(uint64_t x);
static inline uint64_t E_512_1(uint64_t x);
static inline uint64_t s_512_0(uint64_t x);
static inline uint64_t s_512_1(uint64_t x);
static inline uint64_t to_big_endian(uint64_t w);
static void print_word(uint64_t w);

void sha512(void *message, size_t len, sha512_s *digest)
{
    unsigned i, t;
    uint8_t *msg = message;
    size_t l = len*8, k, nblocks, nbytes;
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
    
    /*
        Padding -Solve for k such that l+1+k is congruent to 896 mod 1024
     */
    k = (896 - (l + 1))%1024;
    nbytes = (l + 1 + k + 128)/8 + !!((l + 1 + k + 128) % 8);
    nblocks = nbytes/128 + !!(nbytes % 128);

    states = allocz(nblocks * sizeof(*states));
    
    for(i = 0, stptr = states; i < len; i++) {
        if(!(i % 128) && i != 0)
            stptr++;
        stptr->b[i % 128] = msg[i];
    }
    if(!(i % 128) && i != 0) {
        stptr++;
    }
    stptr->b[i % 128] |= 0x80;
    i++;
    while(i < nbytes) {
        if(!(i % 128) && i != 0)
            stptr++;
        i++;
    }

    /* 
     Add size so that its representation is guaranteed to be big endian. 
     Size does not conform to standards since a 128-bit integer is required.
     */
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

#define INNER_ROUND()   T1 = h + E_512_1(e) + Ch(e, f, g) + K[t] + W[t];    \
                        T2 = E_512_0(a) + Maj(a, b, c); \
                        h = g;  g = f;  \
                        f = e;  e = d + T1; \
                        d = c;  c = b;  \
                        b = a;  a = T1 + T2;    \
                        t++
    
        /* Many tests showed some loop unrolling brought performance increase */
        t = 0;
        while(t < 80) {
            INNER_ROUND();
            INNER_ROUND();
            INNER_ROUND();
            INNER_ROUND();
        }
#undef INNER_ROUND

        H[0] += a;  H[1] += b;
        H[2] += c;  H[3] += d;
        H[4] += e;  H[5] += f;
        H[6] += g;  H[7] += h;
    }
    stptr = states;
    
    /* Zero out plaintext from memory */
    for(t = 0; t < 80; t++) {
        ((volatile uint64_t *)W)[t] = 0llu;
        t++;
        ((volatile uint64_t *)W)[t] = 0llu;
        t++;
        ((volatile uint64_t *)W)[t] = 0llu;
        t++;
        ((volatile uint64_t *)W)[t] = 0llu;
    }
    for(i = 0; i < nblocks; i++)
        zero_block(&stptr[i]);
    free(states);
}

void zero_block(volatile block_s *block)
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
    uint8_t *pres = (uint8_t *)&res;
    
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

int sha512_equal(sha512_s *d1, sha512_s *d2)
{
    unsigned i;
    uint64_t *a = d1->word, *b = d2->word;
    
    for(i = 0; i < sizeof(d1->word)/sizeof(uint64_t); i++) {
        if(*a++ != *b++)
            return 0;
    }
    return 1;
}

void PBKDF(char *pass, salt_s salt, unsigned C, size_t kLen)
{
    unsigned i, Ti, U0;
    size_t r, len;
    
    len = (kLen / sizeof(uint64_t[8])) + !!(kLen / sizeof(uint64_t[8]));
    r = kLen - (len - 1) * (sizeof(uint64_t[8]));
    
    for(i = 1; i <= len; i++) {
        Ti = 0;
        //U0 =
    }
}

/*
 Salt generator
 ,,|,, standards
 */
salt_s get_salt(void)
{
    salt_s salt;
    
    srand((int)time(NULL));
    
    /*
     rand() tends to ignore the signed bit despite it returning
     type 'int'. Assign only the lower 16 bits in 4ths for
     better entropy.
     */
    salt.quad[0] = rand();
    salt.quad[1] = rand();
    salt.quad[2] = rand();
    salt.quad[3] = rand();
    
    return salt;
}

/*AES Implementation */

typedef union word_u word_u;

union word_u
{
    uint32_t word;
    uint8_t b[4];
};

static uint8_t sbox[16][16] = {
    { 0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76 },
    { 0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0 },
    { 0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15 },
    { 0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75 },
    { 0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84 },
    { 0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf },
    { 0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8 },
    { 0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2 },
    { 0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73 },
    { 0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb },
    { 0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79 },
    { 0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08 },
    { 0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a },
    { 0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e },
    { 0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf },
    { 0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 }
};

static union {
    uint8_t b[Nb][4];
    uint16_t s[Nb][2];
    uint32_t w[Nb];
    word_u word[Nb];
}
state;

static inline uint8_t SubBytes(uint8_t b);
static inline word_u SubWord(word_u word);
static inline word_u RotWord(word_u word);
static inline void ShiftRows(void);
static inline void MixColumns(void);
static inline void AddRoundKey(void);

static inline void KeyExpansion(uint8_t *key, word_u *w);

aes_digest_s *aes_encrypt(void *message, size_t len)
{
    
    aes_digest_s *digest;

   
    return digest;
}

void aes_block_encrypt(void *message, aesblock_s *block)
{
    state.b[0][0] = 1;
}

inline uint8_t SubBytes(uint8_t b)
{
    return sbox[b >> 4][b & 0x0f];
}

inline word_u SubWord(word_u word)
{
    return (word_u) {
        .b[0] = SubBytes(word.b[0]),
        .b[1] = SubBytes(word.b[1]),
        .b[2] = SubBytes(word.b[2]),
        .b[3] = SubBytes(word.b[3])
    };
}

inline word_u RotWord(word_u word)
{
    uint8_t backup = word.b[0];
    
    word.word = word.word >> 8;
    word.b[3] = backup;
    return word;
}

inline void ShiftRows(void)
{
    union {
        uint8_t _8;
        uint16_t _16;
    }backup;

    
    backup._8 = state.b[Nk-3][0];
    state.w[Nk-3] = state.w[Nk-3] >> 8;
    state.b[Nk-3][3] = backup._16;
    
    backup._16 = state.s[Nk-2][0];
    state.w[Nk-2] = state.w[Nk-2] >> 16;
    state.s[Nk-2][1] = backup._16;
    
    backup._8 = state.b[Nk-1][3];
    state.w[Nk-1] = state.w[Nk-1] << 8;
    state.b[Nk-1][0] = backup._8;
}

inline void MixColumns(void)
{
    
}

inline void AddRoundKey(void)
{
    
}

inline void KeyExpansion(uint8_t *key, word_u *w)
{
    unsigned i = 0;
    word_u temp;
    
    for(i = 0; i < Nk; i++) {
        w[i].b[0] = key[4*i+0];
        w[i].b[1] = key[4*i+1];
        w[i].b[2] = key[4*i+2];
        w[i].b[3] = key[4*i+3];
    }
    
    i = Nk;
    
    while(i < Nb*(Nr+1)) {
        temp = w[i-1];
        
        if(!(i % Nk))
            ;// temp =SubWord(<#word_u word#>)
            
    }
}