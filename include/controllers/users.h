#ifndef _POCKET_USERS_H_
#define _POCKET_USERS_H_

#include "models/user.h"

#define DEFAULT_USERS_POOL_INIT			16

extern unsigned int pocket_users_init (void);

extern void pocket_users_end (void);

extern User *pocket_user_get (void);

extern User *pocket_user_get_by_email (const char *email);

extern bool pocket_user_check_by_email (const char *email);

// {
//   "email": "erick.salas@ermiry.com",
//   "iat": 1596532954
//   "id": "5eb2b13f0051f70011e9d3af",
//   "name": "Erick Salas",
//   "role": "god",
//   "username": "erick"
// }
extern void *pocket_user_parse_from_json (void *user_json_ptr);

extern void pocket_user_delete (void *user_ptr);

#endif