#ifndef DATABASE_H_
#define DATABASE_H_

#ifdef __cplusplus
extern "C" {
#endif

extern void db_init(const char *name);

extern void store_account(void);
extern void authenticate(void);

#ifdef __cplusplus
}
#endif

#endif
