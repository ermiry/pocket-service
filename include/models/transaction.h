#ifndef _MODELS_TRANSACTION_H_
#define _MODELS_TRANSACTION_H_

#include <time.h>

#include <bson/bson.h>
#include <mongoc/mongoc.h>

#include <cerver/types/types.h>

#define TRANSACTIONS_COLL_NAME         	"pocket.transactions"

#define	TRANSACTION_ID_SIZE				32
#define	TRANSACTION_CATEGORY_SIZE		32
#define TRANSACTION_TITLE_SIZE			1024

#define TRANSACTION_BUFFER_SIZE			128

extern unsigned int transactions_model_init (void);

extern void transactions_model_end (void);

#define TRANS_TYPE_MAP(XX)					\
	XX(0,	NONE, 		None)				\
	XX(1,	SINGLE, 	Single)				\
	XX(2,	RECURRENT, 	Recurrent)

typedef enum TransType {

	#define XX(num, name, string) TRANS_TYPE_##name = num,
	TRANS_TYPE_MAP (XX)
	#undef XX

} TransType;

extern const char *trans_type_to_string (const TransType type);

typedef struct Transaction {

	// transaction's unique id
	bson_oid_t oid;
	char id[TRANSACTION_ID_SIZE];

	// reference to the owner of this transaction
	bson_oid_t user_oid;

	// reference to the owener's defined category
	bson_oid_t category_oid;
	char category_id[TRANSACTION_CATEGORY_SIZE];

	// reference to the place where this transaction was made
	bson_oid_t place_oid;

	// reference to the payment method that was used
	// to create this transaction
	bson_oid_t payment_oid;

	// reference to the currency the transaction was made on
	bson_oid_t currency_oid;

	// the name of the transaction
	// is given by the user and displayed in the app
	char title[TRANSACTION_TITLE_SIZE];

	// the actual value of the transaction
	double amount;

	// when the transaction was made
	time_t date;

	// a transaction can be of any of these types
	// single -> mae only once, like buying a coffe
	// recurrent -> made every x amount of time, like a subscription
	TransType type;

} Transaction;

extern void *transaction_new (void);

extern void transaction_delete (void *transaction_ptr);

extern void transaction_print (const Transaction *transaction);

extern unsigned int transaction_get_by_oid (
	Transaction *trans, const bson_oid_t *oid, const bson_t *query_opts
);

extern unsigned int transaction_get_by_oid_and_user (
	Transaction *trans,
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts
);

extern unsigned int transaction_get_by_oid_and_user_to_json (
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts,
	char **json, size_t *json_len
);

// get all the transactions that are related to a user
extern mongoc_cursor_t *transactions_get_all_by_user (
	const bson_oid_t *user_oid, const bson_t *opts
);

extern unsigned int transactions_get_all_by_user_to_json (
	const bson_oid_t *user_oid, const bson_t *opts,
	char **json, size_t *json_len
);

extern unsigned int transaction_insert_one (
	const Transaction *transaction
);

extern unsigned int transaction_update_one (
	const Transaction *transaction
);

extern unsigned int transaction_delete_one_by_oid_and_user (
	const bson_oid_t *oid, const bson_oid_t *user_oid
);

#endif