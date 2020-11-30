#ifndef _POCKET_USERS_H_
#define _POCKET_USERS_H_

#include <bson/bson.h>

#include <cerver/collections/dlist.h>

#include "models/user.h"

#define DEFAULT_USERS_POOL_INIT			16

struct _HttpReceive;
struct _HttpResponse;

extern const bson_t *user_login_query_opts;
extern DoubleList *user_login_select;

extern const bson_t *user_transactions_query_opts;
extern DoubleList *user_transactions_select;

extern const bson_t *user_categories_query_opts;
extern DoubleList *user_categories_select;

extern struct _HttpResponse *users_works;
extern struct _HttpResponse *missing_user_values;
extern struct _HttpResponse *wrong_password;
extern struct _HttpResponse *user_not_found;
extern struct _HttpResponse *repeated_email;

extern unsigned int pocket_users_init (void);

extern void pocket_users_end (void);

extern User *pocket_user_create (
	const char *name,
	const char *username,
	const char *email,
	const char *password,
	const bson_oid_t *role_oid
);

extern User *pocket_user_get_by_email (const String *email);

extern u8 pocket_user_check_by_email (
	const struct _HttpReceive *http_receive, const String *email
);

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