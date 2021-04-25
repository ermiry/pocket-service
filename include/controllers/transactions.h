#ifndef _POCKET_TRANSACTIONS_H_
#define _POCKET_TRANSACTIONS_H_

#include <bson/bson.h>

#include <cerver/collections/pool.h>

#include "errors.h"

#include "models/transaction.h"
#include "models/user.h"

#define DEFAULT_TRANS_POOL_INIT			32

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

extern void pocket_trans_delete (void *trans_ptr);

#endif