#ifndef _POCKET_USERS_H_
#define _POCKET_USERS_H_

#include <bson/bson.h>

#include <cerver/collections/dlist.h>

#define DEFAULT_USERS_POOL_INIT			16

struct _HttpReceive;
struct _HttpRequest;

#pragma region main

extern const bson_t *user_transactions_query_opts;
extern DoubleList *user_transactions_select;

extern const bson_t *user_categories_query_opts;
extern DoubleList *user_categories_select;

extern unsigned int pocket_users_init (void);

extern void pocket_users_end (void);

// {
//   "email": "erick.salas@ermiry.com",
//   "iat": 1596532954
//   "id": "5eb2b13f0051f70011e9d3af",
//   "name": "Erick Salas",
//   "role": "god",
//   "username": "erick",
// }
extern void *pocket_user_parse_from_json (void *user_json_ptr);

extern void pocket_user_delete (void *user_ptr);

#pragma endregion

#pragma region routes

// GET /api/users/
extern void users_handler (
    const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// POST /api/users/login
extern void users_login_handler (
    const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// POST /api/users/register
extern void users_register_handler (
    const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

#pragma endregion

#endif