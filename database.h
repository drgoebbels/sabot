#ifndef DATABASE_H_
#define DATABASE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sanet.h"

extern void db_init(const char *name);

extern void dbadd_user_record(user_s *user, const char *server, time_t enter);
extern void dblog_message(message_s *msg);

extern void store_account(void);
extern void authenticate(void);

extern inline void db_clean(void);

#ifdef __cplusplus
}
#endif

#endif
