#ifndef _POCKET_TRANSACTIONS_H_
#define _POCKET_TRANSACTIONS_H_

#include <bson/bson.h>

#include <cerver/collections/pool.h>

#include "errors.h"

#include "models/transaction.h"
#include "models/user.h"

#define TRANS_POOL_INIT			32

struct _HttpResponse;

extern Pool *trans_pool;

extern const bson_t *trans_no_user_query_opts;

extern struct _HttpResponse *no_user_trans;

extern struct _HttpResponse *trans_created_success;
extern struct _HttpResponse *trans_created_bad;
extern struct _HttpResponse *trans_deleted_success;
extern struct _HttpResponse *trans_deleted_bad;

extern unsigned int pocket_trans_init (void);

extern void pocket_trans_end (void);

extern unsigned int pocket_trans_get_all_by_user (
	const bson_oid_t *user_oid,
	char **json, size_t *json_len
);

extern Transaction *pocket_trans_get_by_id_and_user (
	const String *trans_id, const bson_oid_t *user_oid
);

extern u8 pocket_trans_get_by_id_and_user_to_json (
	const char *trans_id, const bson_oid_t *user_oid,
	const bson_t *query_opts,
	char **json, size_t *json_len
);

extern PocketError pocket_trans_create (
	const User *user, const String *request_body
);

extern PocketError pocket_trans_update (
	const User *user, const String *trans_id,
	const String *request_body
);

extern PocketError pocket_trans_delete (
	const User *user, const String *trans_id
);

extern void pocket_trans_return (void *trans_ptr);

#endif