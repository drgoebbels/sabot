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

extern void print_sha512digest(sha512_s *digest);

extern void PBKDF(char *pass, salt_s salt, unsigned C, size_t kLen);

extern salt_s get_salt(void);
    
    
    
/*************** AES Implementation ***************/

#define AES_MODE_ECB 1  /*
                            If this isn't set, plaintext size must be x128 bytes
                            or else the behavior is undefined.
                        */
    
#define AES_MODE_CBC 2

#define AES_MODE (AES_MODE_ECB | AES_MODE_CBC)
    
#define AES_128 128
#define AES_192 192
#define AES_256 256

#define AES_BLOCK_LENGTH 128
#define AES_BLOCK_BYTELEN (AES_BLOCK_LENGTH/8)

#define KEY_LENGTH AES_128
#define Nb (AES_BLOCK_LENGTH/32)
#define Nk (KEY_LENGTH/(4*8))
    
//#define STATIC_RCON

#if KEY_LENGTH == AES_128
    #define Nr 10
#elif KEY_LENGTH == AES_192
    #define Nr 12
#elif KEY_LENGTH == AES_256
    #define Nr 14
#else
    #error "Invalid Key Length"
#endif
    
typedef union aesblock_s aesblock_s;
    
typedef struct aes_digest_s aes_digest_s;

/* 
 Naming of union members correspond with naming conventions for 
 addressable intel units. 
 b = byte (8 bits)
 w = word (16 bits)
 l = long word (32 bits)
 q = quad word (64 bits)
 */
union aesblock_s
{
    uint8_t b[4][Nb];
    uint16_t w[4][Nb/2];
    uint32_t l[4];
    uint64_t q[2];
};
    
struct aes_digest_s
{
    size_t size;
    aesblock_s data[];
};

extern aes_digest_s *aes_encrypt(void *message, size_t len, char *key);
extern aes_digest_s *aes_decrypt(void *message, size_t len, char *key);

extern void print_block(aesblock_s *b);
extern void print_aesdigest(aes_digest_s *digest);
    
#ifdef __cplusplus
}
#endif

#endif
