#ifndef _MODELS_TRANSACTION_H_
#define _MODELS_TRANSACTION_H_

#include <time.h>

#include <mongoc/mongoc.h>
#include <bson/bson.h>

#include <cerver/types/types.h>

#define TRANSACTION_TITLE_LEN			1024

extern mongoc_collection_t *transactions_collection;

// opens handle to transaction collection
extern unsigned int transactions_collection_get (void);

extern void transactions_collection_close (void);

typedef struct Transaction {

	bson_oid_t oid;

	bson_oid_t user_oid;

	char title[TRANSACTION_TITLE_LEN];
	double amount;
	time_t date;

} Transaction;

extern void *transaction_new (void);

extern void transaction_delete (void *transaction_ptr);

extern void transaction_print (Transaction *transaction);

extern bson_t *transaction_query_oid (const bson_oid_t *oid);

extern const bson_t *transaction_find_by_oid (
	const bson_oid_t *oid, const bson_t *query_opts
);

extern u8 transaction_get_by_oid (
	Transaction *trans, const bson_oid_t *oid, const bson_t *query_opts
);

extern const bson_t *transaction_find_by_oid_and_user (
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts
);

extern u8 transaction_get_by_oid_and_user (
	Transaction *trans,
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts
);

extern bson_t *transaction_to_bson (Transaction *trans);

extern bson_t *transaction_update_bson (Transaction *trans);

// get all the transactions that are related to a user
extern mongoc_cursor_t *transactions_get_all_by_user (
	const bson_oid_t *user_oid, const bson_t *opts
);

#endif