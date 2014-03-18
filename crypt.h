#ifndef CRYPT_H_
#define CRYPT_H_

#ifdef __cplusplus
extern "C" {
#endif

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

extern void PBKDF(char *pass, salt_s salt, unsigned C, size_t kLen);

extern salt_s get_salt(void);
    
    
    
/*************** AES Implementation ***************/
#define BLOCK_LENGTH 128
#define Nb BLOCK_LENGTH/32
#define Nk BLOCK_LENGTH/(4*8)
    
#define STATIC_RCON

#define AES_BLOCK_128 128
#define AES_BLOCK_192 192
#define AES_BLOCK_256 256

#if BLOCK_LENGTH == AES_BLOCK_128
    #define Nr 10
#elif BLOCK_LENGTH == AES_BLOCK_192
    #define Nr 12
#elif BLOCK_LENGTH == AES_BLOCK_256
    #define Nr 14
#else
    #error "Invalid Block Length"
#endif
    
typedef struct aes_digest_s aes_digest_s;
typedef union aesblock_s aesblock_s;

    
struct aes_digest_s
{
    size_t len;
    uint8_t data[];
};
    
union aesblock_s
{
    uint8_t state[Nb][4];
    uint32_t word[Nb];
};

extern aes_digest_s *aes_encrypt(void *message, size_t len);

extern void aes_block_encrypt(void *message, aesblock_s *block);
    
#ifdef __cplusplus
}
#endif

#endif
