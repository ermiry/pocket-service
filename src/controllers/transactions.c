#include <stdlib.h>

#include <time.h>

#include <cerver/types/string.h>

#include <cerver/collections/pool.h>

#include <cerver/http/response.h>
#include <cerver/http/json/json.h>

#include <cerver/utils/log.h>

#include <cmongo/crud.h>
#include <cmongo/select.h>

#include "errors.h"

#include "models/transaction.h"
#include "models/user.h"

#include "controllers/transactions.h"

Pool *trans_pool = NULL;

const bson_t *trans_no_user_query_opts = NULL;
static CMongoSelect *trans_no_user_select = NULL;

HttpResponse *no_user_trans = NULL;

HttpResponse *trans_created_success = NULL;
HttpResponse *trans_created_bad = NULL;
HttpResponse *trans_deleted_success = NULL;
HttpResponse *trans_deleted_bad = NULL;

void pocket_trans_return (void *trans_ptr);

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

	trans_no_user_select = cmongo_select_new ();
	(void) cmongo_select_insert_field (trans_no_user_select, "title");
	(void) cmongo_select_insert_field (trans_no_user_select, "amount");
	(void) cmongo_select_insert_field (trans_no_user_select, "date");
	(void) cmongo_select_insert_field (trans_no_user_select, "category");

	trans_no_user_query_opts = mongo_find_generate_opts (trans_no_user_select);

	if (trans_no_user_query_opts) retval = 0;

	return retval;

}

static unsigned int pocket_trans_init_responses (void) {

	unsigned int retval = 1;

	no_user_trans = http_response_json_key_value (
		HTTP_STATUS_NOT_FOUND, "msg", "Failed to get user's transaction(s)"
	);

	trans_created_success = http_response_json_key_value (
		HTTP_STATUS_OK, "oki", "doki"
	);

	trans_created_bad = http_response_json_key_value (
		HTTP_STATUS_BAD_REQUEST, "error", "Failed to create transaction!"
	);

	trans_deleted_success = http_response_json_key_value (
		HTTP_STATUS_OK, "oki", "doki"
	);

	trans_deleted_bad = http_response_json_key_value (
		HTTP_STATUS_BAD_REQUEST, "error", "Failed to delete transaction!"
	);

	if (
		no_user_trans
		&& trans_created_success && trans_created_bad
		&& trans_deleted_success && trans_deleted_bad
	) retval = 0;

	return retval;

}

unsigned int pocket_trans_init (void) {

	unsigned int errors = 0;

	errors |= pocket_trans_init_pool ();

	errors |= pocket_trans_init_query_opts ();

	errors |= pocket_trans_init_responses ();

	return errors;

}

void pocket_trans_end (void) {

	cmongo_select_delete (trans_no_user_select);
	bson_destroy ((bson_t *) trans_no_user_query_opts);

	pool_delete (trans_pool);
	trans_pool = NULL;

	http_response_delete (no_user_trans);

	http_response_delete (trans_created_success);
	http_response_delete (trans_created_bad);
	http_response_delete (trans_deleted_success);
	http_response_delete (trans_deleted_bad);

}

unsigned int pocket_trans_get_all_by_user (
	const bson_oid_t *user_oid,
	char **json, size_t *json_len
) {

	return transactions_get_all_by_user_to_json (
		user_oid, trans_no_user_query_opts,
		json, json_len
	);

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
				pocket_trans_return (trans);
				trans = NULL;
			}
		}
	}

	return trans;

}

u8 pocket_trans_get_by_id_and_user_to_json (
	const char *trans_id, const bson_oid_t *user_oid,
	const bson_t *query_opts,
	char **json, size_t *json_len
) {

	u8 retval = 1;

	if (trans_id) {
		bson_oid_t trans_oid = { 0 };
		bson_oid_init_from_string (&trans_oid, trans_id);

		retval = transaction_get_by_oid_and_user_to_json (
			&trans_oid, user_oid,
			query_opts,
			json, json_len
		);
	}

	return retval;

}

static Transaction *pocket_trans_create_actual (
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

		if (title) (void) strncpy (trans->title, title, TRANSACTION_TITLE_SIZE - 1);
		trans->amount = amount;

		if (category_id) {
			bson_oid_init_from_string (&trans->category_oid, category_id);
		}

		if (date) {
			int y = 0, M = 0, d = 0, h = 0, m = 0;
			float s = 0;
			(void) sscanf (date, "%d-%d-%dT%d:%d:%f", &y, &M, &d, &h, &m, &s);

			struct tm date;
			date.tm_year = y - 1900;	// Year since 1900
			date.tm_mon = M - 1;		// 0-11
			date.tm_mday = d;			// 1-31
			date.tm_hour = h;			// 0-23
			date.tm_min = m;			// 0-59
			date.tm_sec = (int) s;		// 0-61 (0-60 in C++11)

			trans->date = mktime (&date);
		}

		else {
			trans->date = time (NULL);
		}
	}

	return trans;

}

static void pocket_trans_parse_json (
	json_t *json_body,
	const char **title,
	double *amount,
	const char **category,
	const char **place,
	const char **date
) {

	// get values from json to create a new transaction
	const char *key = NULL;
	json_t *value = NULL;
	if (json_typeof (json_body) == JSON_OBJECT) {
		json_object_foreach (json_body, key, value) {
			if (!strcmp (key, "title")) {
				*title = json_string_value (value);
				#ifdef POCKET_DEBUG
				(void) printf ("title: \"%s\"\n", *title);
				#endif
			}

			else if (!strcmp (key, "amount")) {
				*amount = json_real_value (value);
				#ifdef POCKET_DEBUG
				(void) printf ("amount: %f\n", *amount);
				#endif
			}

			else if (!strcmp (key, "category")) {
				*category = json_string_value (value);
				#ifdef POCKET_DEBUG
				(void) printf ("category: \"%s\"\n", *category);
				#endif
			}

			else if (!strcmp (key, "place")) {
				*place = json_string_value (value);
				#ifdef POCKET_DEBUG
				(void) printf ("place: \"%s\"\n", *place);
				#endif
			}

			else if (!strcmp (key, "date")) {
				*date = json_string_value (value);
				#ifdef POCKET_DEBUG
				(void) printf ("date: \"%s\"\n", *date);
				#endif
			}
		}
	}

}

static PocketError pocket_trans_create_parse_json (
	Transaction **trans,
	const char *user_id, const String *request_body
) {

	PocketError error = POCKET_ERROR_NONE;

	const char *title = NULL;
	double amount = 0;
	const char *category_id = NULL;
	const char *place_id = NULL;
	const char *date = NULL;

	json_error_t json_error =  { 0 };
	json_t *json_body = json_loads (request_body->str, 0, &json_error);
	if (json_body) {
		pocket_trans_parse_json (
			json_body,
			&title, &amount,
			&category_id, &place_id, &date
		);

		if (title && category_id) {
			*trans = pocket_trans_create_actual (
				user_id,
				title, amount,
				category_id,
				date
			);

			if (*trans == NULL) error = POCKET_ERROR_SERVER_ERROR;
		}

		else {
			error = POCKET_ERROR_MISSING_VALUES;
		}

		json_decref (json_body);
	}

	else {
		cerver_log_error (
			"json_loads () - json error on line %d: %s\n",
			json_error.line, json_error.text
		);

		error = POCKET_ERROR_BAD_REQUEST;
	}

	return error;

}

PocketError pocket_trans_create (
	const User *user, const String *request_body
) {

	PocketError error = POCKET_ERROR_NONE;

	if (request_body) {
		Transaction *trans = NULL;

		error = pocket_trans_create_parse_json (
			&trans,
			user->id, request_body
		);

		if (error == POCKET_ERROR_NONE) {
			#ifdef POCKET_DEBUG
			transaction_print (trans);
			#endif

			if (!transaction_insert_one (trans)) {
				// update users values
				(void) user_add_transactions (user);
			}

			else {
				error = POCKET_ERROR_SERVER_ERROR;
			}

			pocket_trans_return (trans);
		}
	}

	else {
		#ifdef POCKET_DEBUG
		cerver_log_error ("Missing request body to create transaction!");
		#endif

		error = POCKET_ERROR_BAD_REQUEST;
	}

	return error;

}

static PocketError pocket_trans_update_parse_json (
	Transaction *trans, const String *request_body
) {

	PocketError error = POCKET_ERROR_NONE;

	const char *title = NULL;
	double amount = 0;
	const char *category_id = NULL;
	const char *place_id = NULL;
	const char *date = NULL;

	json_error_t json_error =  { 0 };
	json_t *json_body = json_loads (request_body->str, 0, &json_error);
	if (json_body) {
		pocket_trans_parse_json (
			json_body,
			&title, &amount,
			&category_id, &place_id, &date
		);

		if (title) (void) strncpy (trans->title, title, TRANSACTION_TITLE_SIZE - 1);
		trans->amount = amount;
		if (category_id) (void) bson_oid_init_from_string (&trans->category_oid, category_id);
		if (place_id) (void) bson_oid_init_from_string (&trans->place_oid, place_id);

		json_decref (json_body);
	}

	else {
		#ifdef POCKET_DEBUG
		cerver_log_error (
			"json_loads () - json error on line %d: %s\n",
			json_error.line, json_error.text
		);
		#endif

		error = POCKET_ERROR_BAD_REQUEST;
	}

	return error;

}

PocketError pocket_trans_update (
	const User *user, const String *trans_id,
	const String *request_body
) {

	PocketError error = POCKET_ERROR_NONE;

	if (request_body) {
		Transaction *trans = pocket_trans_get_by_id_and_user (
			trans_id, &user->oid
		);

		if (trans) {
			// get update values
			if (pocket_trans_update_parse_json (
				trans, request_body
			) == POCKET_ERROR_NONE) {
				// update the transaction in the db
				if (transaction_update_one (trans)) {
					error = POCKET_ERROR_SERVER_ERROR;
				}
			}

			pocket_trans_return (trans);
		}

		else {
			#ifdef POCKET_DEBUG
			cerver_log_error ("Failed to get matching transaction!");
			#endif

			error = POCKET_ERROR_NOT_FOUND;
		}
	}

	else {
		#ifdef POCKET_DEBUG
		cerver_log_error ("Missing request body to update transaction!");
		#endif

		error = POCKET_ERROR_BAD_REQUEST;
	}

	return error;

}

PocketError pocket_trans_delete (
	const User *user, const String *trans_id
) {

	PocketError error = POCKET_ERROR_NONE;

	bson_oid_t oid = { 0 };
	bson_oid_init_from_string (&oid, trans_id->str);

	if (!transaction_delete_one_by_oid_and_user (
		&oid, &user->oid
	)) {
		#ifdef POCKET_DEBUG
		cerver_log_debug ("Deleted transaction %s", trans_id->str);
		#endif
	}

	else {
		error = POCKET_ERROR_BAD_REQUEST;
	}

	return error;

}

void pocket_trans_return (void *trans_ptr) {

	(void) memset (trans_ptr, 0, sizeof (Transaction));
	(void) pool_push (trans_pool, trans_ptr);

}