#include <stdio.h>

#include "crypt.h"

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