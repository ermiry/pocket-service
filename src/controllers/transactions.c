#include <stdlib.h>

#include <time.h>

#include <cerver/types/string.h>

#include <cerver/collections/pool.h>

#include <cerver/http/json/json.h>

#include <cerver/utils/log.h>

#include "mongo.h"

#include "controllers/transactions.h"

#include "models/transaction.h"

Pool *trans_pool = NULL;

const bson_t *trans_no_user_query_opts = NULL;
DoubleList *trans_no_user_select = NULL;

void pocket_trans_delete (void *trans_ptr);

static unsigned int pocket_trans_init_pool (void) {

	unsigned int retval = 1;

	trans_pool = pool_create (transaction_delete);
	if (trans_pool) {
		pool_set_create (trans_pool, transaction_new);
		pool_set_produce_if_empty (trans_pool, true);
		if (!pool_init (trans_pool, transaction_new, DEFAULT_TRANS_POOL_INIT)) {
			retval = 0;
		}

		else {
			cerver_log_error ("Failed to init trans pool!");
		}
	}

	else {
		cerver_log_error ("Failed to create trans pool!");
	}

	return retval;

}

static unsigned int pocket_trans_init_query_opts (void) {

	unsigned int retval = 1;

	trans_no_user_select = dlist_init (str_delete, str_comparator);
	(void) dlist_insert_at_end_unsafe (trans_no_user_select, str_new ("title"));
	(void) dlist_insert_at_end_unsafe (trans_no_user_select, str_new ("amount"));
	(void) dlist_insert_at_end_unsafe (trans_no_user_select, str_new ("date"));
	(void) dlist_insert_at_end_unsafe (trans_no_user_select, str_new ("category"));

	trans_no_user_query_opts = mongo_find_generate_opts (trans_no_user_select);

	if (trans_no_user_query_opts) retval = 0;

	return retval;

}

unsigned int pocket_trans_init (void) {

	unsigned int errors = 0;

	errors |= pocket_trans_init_pool ();

	errors |= pocket_trans_init_query_opts ();

	return errors;

}

void pocket_trans_end (void) {

	bson_destroy ((bson_t *) trans_no_user_query_opts);

	pool_delete (trans_pool);
	trans_pool = NULL;

}

Transaction *pocket_trans_get_by_id_and_user (
	const String *trans_id, const bson_oid_t *user_oid
) {

	Transaction *trans = NULL;

	if (trans_id) {
		trans = (Transaction *) pool_pop (trans_pool);
		if (trans) {
			bson_oid_init_from_string (&trans->oid, trans_id->str);

			if (transaction_get_by_oid_and_user (
				trans,
				&trans->oid, user_oid,
				NULL
			)) {
				pocket_trans_delete (trans);
				trans = NULL;
			}
		}
	}

	return trans;

}

Transaction *pocket_trans_create (
	const char *user_id,
	const char *title,
	const double amount,
	const char *category_id,
	const char *date
) {

	Transaction *trans = (Transaction *) pool_pop (trans_pool);
	if (trans) {
		bson_oid_init (&trans->oid, NULL);

		bson_oid_init_from_string (&trans->user_oid, user_id);

		if (title) (void) strncpy (trans->title, title, TRANSACTION_TITLE_LEN);
		trans->amount = amount;

		if (category_id) {
			bson_oid_init_from_string (&trans->category_oid, category_id);
		}

		if (date) trans->date = (time_t) atol (date);
		else trans->date = time (NULL);
	}

	return trans;

}

void pocket_trans_delete (void *trans_ptr) {

	(void) memset (trans_ptr, 0, sizeof (Transaction));
	(void) pool_push (trans_pool, trans_ptr);

}