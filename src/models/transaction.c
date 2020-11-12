#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <cerver/types/types.h>

#include <cerver/utils/log.h>

#include "mongo.h"

#include "models/transaction.h"

#define TRANSACTIONS_COLL_NAME         				"transactions"

mongoc_collection_t *transactions_collection = NULL;

// opens handle to transaction collection
unsigned int transactions_collection_get (void) {

	unsigned int retval = 1;

	transactions_collection = mongo_collection_get (TRANSACTIONS_COLL_NAME);
	if (transactions_collection) {
		cerver_log_debug ("Opened handle to transactions collection!");
		retval = 0;
	}

	else {
		cerver_log_error ("Failed to get handle to transactions collection!");
	}

	return retval;

}

void transactions_collection_close (void) {

	if (transactions_collection) mongoc_collection_destroy (transactions_collection);

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

static void trans_doc_parse (Transaction *trans, const bson_t *trans_doc) {

	bson_iter_t iter = { 0 };
	if (bson_iter_init (&iter, trans_doc)) {
		char *key = NULL;
		bson_value_t *value = NULL;
		while (bson_iter_next (&iter)) {
			key = (char *) bson_iter_key (&iter);
			value = (bson_value_t *) bson_iter_value (&iter);

			if (!strcmp (key, "_id"))
				bson_oid_copy (&value->value.v_oid, &trans->oid);

			else if (!strcmp (key, "user"))
				bson_oid_copy (&value->value.v_oid, &trans->user_oid);

			else if (!strcmp (key, "title") && value->value.v_utf8.str) 
				(void) strncpy (trans->title, value->value.v_utf8.str, TRANSACTION_TITLE_LEN);

			else if (!strcmp (key, "amount")) 
				trans->amount = value->value.v_double;

			else if (!strcmp (key, "date")) 
				trans->date = (time_t) bson_iter_date_time (&iter) / 1000;
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

const bson_t *transaction_find_by_oid (
	const bson_oid_t *oid, const bson_t *query_opts
) {

	const bson_t *retval = NULL;

	bson_t *trans_query = bson_new ();
	if (trans_query) {
		(void) bson_append_oid (trans_query, "_id", -1, oid);
		retval = mongo_find_one_with_opts (transactions_collection, trans_query, query_opts);
	}

	return retval;

}

u8 transaction_get_by_oid (
	Transaction *trans, const bson_oid_t *oid, const bson_t *query_opts
) {

	u8 retval = 1;

	if (trans) {
		const bson_t *trans_doc = transaction_find_by_oid (oid, query_opts);
		if (trans_doc) {
			trans_doc_parse (trans, trans_doc);
			bson_destroy ((bson_t *) trans_doc);

			retval = 0;
		}
	}

	return retval;

}

const bson_t *transaction_find_by_oid_and_user (
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts
) {

	const bson_t *retval = NULL;

	bson_t *trans_query = bson_new ();
	if (trans_query) {
		(void) bson_append_oid (trans_query, "_id", -1, oid);
		(void) bson_append_oid (trans_query, "user", -1, user_oid);

		retval = mongo_find_one_with_opts (transactions_collection, trans_query, query_opts);
	}

	return retval;

}

u8 transaction_get_by_oid_and_user (
	Transaction *trans,
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts
) {

	u8 retval = 1;

	if (trans) {
		const bson_t *trans_doc = transaction_find_by_oid_and_user (oid, user_oid, query_opts);
		if (trans_doc) {
			trans_doc_parse (trans, trans_doc);
			bson_destroy ((bson_t *) trans_doc);

			retval = 0;
		}
	}

	return retval;

}

bson_t *transaction_to_bson (Transaction *trans) {

    bson_t *doc = NULL;

    if (trans) {
        doc = bson_new ();
        if (doc) {
            (void) bson_append_oid (doc, "_id", -1, &trans->oid);

			(void) bson_append_oid (doc, "user", -1, &trans->user_oid);

			(void) bson_append_utf8 (doc, "title", -1, trans->title, -1);
			(void) bson_append_double (doc, "amount", -1, trans->amount);
			(void) bson_append_date_time (doc, "date", -1, trans->date * 1000);
        }
    }

    return doc;

}

bson_t *transaction_update_bson (Transaction *trans) {

	bson_t *doc = NULL;

    if (trans) {
        doc = bson_new ();
        if (doc) {
			bson_t set_doc = { 0 };
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
				transactions_collection,
				query, opts
			);
		}
	}

	return retval;

}