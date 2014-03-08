#include <stdio.h>

#include "crypt.h"

typedef struct state_s state_s;

struct state_s
{
    union {
        uint8_t b[128];
        uint64_t word[16];
    };
};

static sha512_s H0 = {
    .word = {
        0x6a09e667f3bcc908lu,
        0xbb67ae8584caa73blu,
        0x3c6ef372fe94f82blu,
        0x510e527fade682d1lu,
        0x9b05688c2b3e6c1flu,
        0x1f83d9abfb41bd6blu,
        0x5be0cd19137e2179lu
    }
};

static inline void zero_state(state_s *state);

void sha512(void *message, size_t len, sha512_s *digest)
{
    unsigned i;
    uint8_t *msg = message;
    size_t l = len*8, div, k;
    state_s state;
    
    zero_state(&state);
    
    div = (l / 1024) ? l % 1024 : l;
    
    /* Solve for k such that l+1+k is congruent to 869 mod 1024 */
    k = (896-(div+1)) % 1024;
    
    for(i = 0; i < len; i++)
        state.b[i] = msg[i];
    state.b[i] |= 0x8000;
    state.word[15] = l;
    
    
}

inline void zero_state(state_s *state)
{
    unsigned i;
    
    for(i = 0; i < 16; i++)
        state->word[i] = 0lu;
}
