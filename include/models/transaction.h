#ifndef _MODELS_TRANSACTION_H_
#define _MODELS_TRANSACTION_H_

#include <time.h>

#include <mongoc/mongoc.h>
#include <bson/bson.h>

#define TRANSACTION_TITLE_LEN			1024

extern mongoc_collection_t *transactions_collection;

// opens handle to transaction collection
extern unsigned int transactions_collection_get (void);

extern void transactions_collection_close (void);

typedef struct Transaction {

	bson_oid_t oid;

	char title[TRANSACTION_TITLE_LEN];
	double amount;
	time_t date;

} Transaction;

extern void *transaction_new (void);

extern void transaction_delete (void *transaction_ptr);

extern void transaction_print (Transaction *transaction);

extern bson_t *transaction_to_bson (Transaction *trans);

#endif