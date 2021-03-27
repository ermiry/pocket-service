#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <cerver/types/types.h>

#include <cerver/utils/log.h>

#include <cmongo/collections.h>
#include <cmongo/crud.h>
#include <cmongo/model.h>

#include "models/transaction.h"

static CMongoModel *transactions_model = NULL;

static void trans_doc_parse (
	void *trans_ptr, const bson_t *trans_doc
);

unsigned int transactions_model_init (void) {

	unsigned int retval = 1;

	transactions_model = cmongo_model_create (TRANSACTIONS_COLL_NAME);
	if (transactions_model) {
		cmongo_model_set_parser (transactions_model, trans_doc_parse);

		retval = 0;
	}

	return retval;

}

void transactions_model_end (void) {

	cmongo_model_delete (transactions_model);

}

const char *trans_type_to_string (TransType type) {

	switch (type) {
		#define XX(num, name, string) case TRANS_TYPE_##name: return #string;
		TRANS_TYPE_MAP(XX)
		#undef XX
	}

	return trans_type_to_string (TRANS_TYPE_NONE);

}

void *transaction_new (void) {

	Transaction *transaction = (Transaction *) malloc (sizeof (Transaction));
	if (transaction) {
		(void) memset (transaction, 0, sizeof (Transaction));
	}

	return transaction;

}

void transaction_delete (void *transaction_ptr) {

	if (transaction_ptr) free (transaction_ptr);

}

void transaction_print (Transaction *transaction) {

	if (transaction) {
		char buffer[128] = { 0 };
		bson_oid_to_string (&transaction->oid, buffer);
		(void) printf ("id: %s\n", buffer);

		(void) printf ("title: %s\n", transaction->title);
		(void) printf ("amount: %.4f\n", transaction->amount);

		(void) strftime (buffer, 128, "%d/%m/%y - %T", gmtime (&transaction->date));
		(void) printf ("date: %s GMT\n", buffer);
	}

}

static void trans_doc_parse (
	void *trans_ptr, const bson_t *trans_doc
) {

	Transaction *trans = (Transaction *) trans_ptr;

	bson_iter_t iter = { 0 };
	if (bson_iter_init (&iter, trans_doc)) {
		char *key = NULL;
		bson_value_t *value = NULL;
		while (bson_iter_next (&iter)) {
			key = (char *) bson_iter_key (&iter);
			value = (bson_value_t *) bson_iter_value (&iter);

			if (!strcmp (key, "_id")) {
				bson_oid_copy (&value->value.v_oid, &trans->oid);
				bson_oid_init_from_string (&trans->oid, trans->id);
			}

			else if (!strcmp (key, "user"))
				bson_oid_copy (&value->value.v_oid, &trans->user_oid);

			else if (!strcmp (key, "category"))
				bson_oid_copy (&value->value.v_oid, &trans->category_oid);

			else if (!strcmp (key, "place"))
				bson_oid_copy (&value->value.v_oid, &trans->place_oid);

			else if (!strcmp (key, "payment"))
				bson_oid_copy (&value->value.v_oid, &trans->payment_oid);

			else if (!strcmp (key, "currency"))
				bson_oid_copy (&value->value.v_oid, &trans->currency_oid);

			else if (!strcmp (key, "title") && value->value.v_utf8.str) {
				(void) strncpy (
					trans->title,
					value->value.v_utf8.str,
					TRANSACTION_TITLE_LEN - 1
				);
			}

			else if (!strcmp (key, "amount")) 
				trans->amount = value->value.v_double;

			else if (!strcmp (key, "date")) 
				trans->date = (time_t) bson_iter_date_time (&iter) / 1000;

			else if (!strcmp (key, "type")) 
				trans->type = (TransType) value->value.v_int32;
		}
	}

}

bson_t *transaction_query_oid (const bson_oid_t *oid) {

	bson_t *query = NULL;

	if (oid) {
		query = bson_new ();
		if (query) {
			(void) bson_append_oid (query, "_id", -1, oid);
		}
	}

	return query;

}

bson_t *transaction_query_by_oid_and_user (
	const bson_oid_t *oid, const bson_oid_t *user_oid
) {

	bson_t *transaction_query = bson_new ();
	if (transaction_query) {
		(void) bson_append_oid (transaction_query, "_id", -1, oid);
		(void) bson_append_oid (transaction_query, "user", -1, user_oid);
	}

	return transaction_query;

}

u8 transaction_get_by_oid (
	Transaction *trans, const bson_oid_t *oid, const bson_t *query_opts
) {

	u8 retval = 1;

	if (trans && oid) {
		bson_t *trans_query = bson_new ();
		if (trans_query) {
			(void) bson_append_oid (trans_query, "_id", -1, oid);
			retval = mongo_find_one_with_opts (
				transactions_model,
				trans_query, query_opts,
				trans
			);
		}
	}

	return retval;

}

u8 transaction_get_by_oid_and_user (
	Transaction *trans,
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts
) {

	u8 retval = 1;

	if (trans && oid && user_oid) {
		bson_t *trans_query = transaction_query_by_oid_and_user (
			oid, user_oid
		);

		if (trans_query) {
			retval = mongo_find_one_with_opts (
				transactions_model,
				trans_query, query_opts,
				trans
			);
		}
	}

	return retval;

}

u8 transaction_get_by_oid_and_user_to_json (
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts,
	char **json, size_t *json_len
) {

	u8 retval = 1;

	if (oid && user_oid) {
		bson_t *trans_query = transaction_query_by_oid_and_user (
			oid, user_oid
		);

		if (trans_query) {
			retval = mongo_find_one_with_opts_to_json (
				transactions_model,
				trans_query, query_opts,
				json, json_len
			);
		}
	}

	return retval;

}

bson_t *transaction_to_bson (const Transaction *trans) {

    bson_t *doc = NULL;

    if (trans) {
        doc = bson_new ();
        if (doc) {
            (void) bson_append_oid (doc, "_id", -1, &trans->oid);

			(void) bson_append_oid (doc, "user", -1, &trans->user_oid);

			(void) bson_append_oid (doc, "category", -1, &trans->category_oid);

			(void) bson_append_utf8 (doc, "title", -1, trans->title, -1);
			(void) bson_append_double (doc, "amount", -1, trans->amount);
			(void) bson_append_date_time (doc, "date", -1, trans->date * 1000);

			(void) bson_append_int32 (doc, "type", -1, trans->type);
        }
    }

    return doc;

}

bson_t *transaction_update_bson (const Transaction *trans) {

	bson_t *doc = NULL;

    if (trans) {
        doc = bson_new ();
        if (doc) {
			bson_t set_doc = BSON_INITIALIZER;
			(void) bson_append_document_begin (doc, "$set", -1, &set_doc);
			(void) bson_append_utf8 (&set_doc, "title", -1, trans->title, -1);
			(void) bson_append_double (&set_doc, "amount", -1, trans->amount);
			// (void) bson_append_date_time (&set_doc, "date", -1, trans->date * 1000);
			(void) bson_append_document_end (doc, &set_doc);
        }
    }

    return doc;

}

// get all the transactions that are related to a user
mongoc_cursor_t *transactions_get_all_by_user (
	const bson_oid_t *user_oid, const bson_t *opts
) {

	mongoc_cursor_t *retval = NULL;

	if (user_oid && opts) {
		bson_t *query = bson_new ();
		if (query) {
			(void) bson_append_oid (query, "user", -1, user_oid);

			retval = mongo_find_all_cursor_with_opts (
				transactions_model,
				query, opts
			);
		}
	}

	return retval;

}

char *transactions_get_all_by_user_to_json (
	const bson_oid_t *user_oid, const bson_t *opts,
	size_t *json_len
) {

	char *json = NULL;

	if (user_oid) {
		bson_t *query = bson_new ();
		if (query) {
			(void) bson_append_oid (query, "user", -1, user_oid);

			json = mongo_find_all_cursor_with_opts_to_json (
				transactions_model,
				query, opts,
				"transactions", json_len
			);
		}
	}

	return json;

}

unsigned int transaction_insert_one (const Transaction *transaction) {

	return mongo_insert_one (
		transactions_model, transaction_to_bson (transaction)
	);

}

unsigned int transaction_update_one (const Transaction *transaction) {

	return mongo_update_one (
		transactions_model,
		transaction_query_oid (&transaction->oid),
		transaction_update_bson (transaction)
	);

}

unsigned int transaction_delete_one_by_oid_and_user (
	const bson_oid_t *oid, const bson_oid_t *user_oid
) {

	unsigned int retval = 1;

	if (oid && user_oid) {
		bson_t *transaction_query = transaction_query_by_oid_and_user (
			oid, user_oid
		);

		if (transaction_query) {
			retval = mongo_delete_one (
				transactions_model, transaction_query
			);
		}
	}

	return retval;

}