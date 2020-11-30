#ifndef _POCKET_TRANSACTIONS_H_
#define _POCKET_TRANSACTIONS_H_

#include <bson/bson.h>

#include <cerver/collections/dlist.h>
#include <cerver/collections/pool.h>

#include "models/transaction.h"

#define DEFAULT_TRANS_POOL_INIT			32

extern Pool *trans_pool;

extern const bson_t *trans_no_user_query_opts;
extern DoubleList *trans_no_user_select;

extern unsigned int pocket_trans_init (void);

extern void pocket_trans_end (void);

extern Transaction *pocket_trans_get_by_id_and_user (
	const String *trans_id, const bson_oid_t *user_oid
);

extern Transaction *pocket_trans_create (
	const char *user_id,
	const char *title,
	const double amount
);

extern void pocket_trans_delete (void *trans_ptr);

#endif