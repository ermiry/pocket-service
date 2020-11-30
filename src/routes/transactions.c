#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cerver/types/types.h>
#include <cerver/types/string.h>

#include <cerver/http/http.h>
#include <cerver/http/route.h>
#include <cerver/http/request.h>
#include <cerver/http/response.h>
#include <cerver/http/json/json.h>

#include <cerver/utils/utils.h>
#include <cerver/utils/log.h>

#include "errors.h"
#include "mongo.h"
#include "pocket.h"

#include "controllers/categories.h"
#include "controllers/transactions.h"
#include "controllers/users.h"

#include "models/category.h"
#include "models/user.h"

static char *pocket_transactions_handler_generate_json (
	User *user,
	mongoc_cursor_t *trans_cursor,
	size_t *json_len
) {

	char *retval = NULL;

	bson_t *doc = bson_new ();
	if (doc) {
		(void) bson_append_int32 (doc, "count", -1, user->trans_count);

		bson_t trans_array = { 0 };
		(void) bson_append_array_begin (doc, "transactions", -1, &trans_array);
		char buf[16] = { 0 };
		const char *key = NULL;
		size_t keylen = 0;

		int i = 0;
		const bson_t *trans_doc = NULL;
		while (mongoc_cursor_next (trans_cursor, &trans_doc)) {
			keylen = bson_uint32_to_string (i, &key, buf, sizeof (buf));
			(void) bson_append_document (&trans_array, key, (int) keylen, trans_doc);

			bson_destroy ((bson_t *) trans_doc);

			i++;
		}
		(void) bson_append_array_end (doc, &trans_array);

		retval = bson_as_relaxed_extended_json (doc, json_len);
	}

	return retval;

}

// GET /api/pocket/transactions
// get all the authenticated user's transactions
void pocket_transactions_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		// get user's transactions from the db
		if (!user_get_by_id (user, user->id, user_transactions_query_opts)) {
			mongoc_cursor_t *trans_cursor = transactions_get_all_by_user (
				&user->oid, trans_no_user_query_opts
			);

			if (trans_cursor) {
				// convert them to json and send them back
				size_t json_len = 0;
				char *json = pocket_transactions_handler_generate_json (
					user, trans_cursor, &json_len
				);

				if (json) {
					(void) http_response_json_custom_reference_send (
						http_receive,
						200,
						json, json_len
					);

					free (json);
				}

				else {
					(void) http_response_send (server_error, http_receive);
				}

				mongoc_cursor_destroy (trans_cursor);
			}

			else {
				(void) http_response_send (no_user_trans, http_receive);
			}
		}

		else {
			(void) http_response_send (bad_user, http_receive);
		}
	}

	else {
		(void) http_response_send (bad_user, http_receive);
	}

}

static void pocket_trans_parse_json (
	json_t *json_body,
	const char **title,
	double *amount,
	const char **category,
	const char **date
) {

	// get values from json to create a new transaction
	const char *key = NULL;
	json_t *value = NULL;
	if (json_typeof (json_body) == JSON_OBJECT) {
		json_object_foreach (json_body, key, value) {
			if (!strcmp (key, "title")) {
				*title = json_string_value (value);
				(void) printf ("title: \"%s\"\n", *title);
			}

			else if (!strcmp (key, "amount")) {
				*amount = json_real_value (value);
				(void) printf ("amount: %f\n", *amount);
			}

			else if (!strcmp (key, "category")) {
				*category = json_string_value (value);
				(void) printf ("category: \"%s\"\n", *category);
			}

			else if (!strcmp (key, "date")) {
				*date = json_string_value (value);
				(void) printf ("date: \"%s\"\n", *date);
			}
		}
	}

}

static PocketError pocket_transaction_create_handler_internal (
	Transaction **trans,
	const char *user_id, const String *request_body
) {

	PocketError error = POCKET_ERROR_NONE;

	if (request_body) {
		const char *title = NULL;
		double amount = 0;
		const char *category_id = NULL;
		const char *date = NULL;

		json_error_t json_error =  { 0 };
		json_t *json_body = json_loads (request_body->str, 0, &json_error);
		if (json_body) {
			pocket_trans_parse_json (
				json_body,
				&title, &amount,
				&category_id, &date
			);

			if (title && (amount != 0)) {
				*trans = pocket_trans_create (
					user_id,
					title, amount
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
	}

	return error;

}

// POST /api/pocket/transactions
// a user has requested to create a new transaction
void pocket_transaction_create_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		Transaction *trans = NULL;

		PocketError error = pocket_transaction_create_handler_internal (
			&trans,
			user->id, request->body
		);

		if (error == POCKET_ERROR_NONE) {
			#ifdef POCKET_DEBUG
			transaction_print (trans);
			#endif

			if (!mongo_insert_one (
				transactions_collection,
				transaction_to_bson (trans)
			)) {
				// update users values
				(void) mongo_update_one (
					users_collection,
					user_query_id (user->id),
					user_create_update_pocket_transactions ()
				);

				// return success to user
				(void) http_response_send (
					trans_created_success,
					http_receive
				);
			}

			else {
				(void) http_response_send (
					trans_created_bad,
					http_receive
				);
			}
			
			pocket_trans_delete (trans);
		}

		else {
			pocket_error_send_response (error, http_receive);
		}
	}

	else {
		(void) http_response_send (bad_user, http_receive);
	}

}

// GET /api/pocket/transactions/:id
// returns information about an existing transaction that belongs to a user
void pocket_transaction_get_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	const String *trans_id = request->params[0];

	User *user = (User *) request->decoded_data;
	if (user) {
		Transaction *trans = (Transaction *) pool_pop (trans_pool);
		if (trans) {
			bson_oid_init_from_string (&trans->oid, trans_id->str);
			bson_oid_init_from_string (&trans->user_oid, user->id);

			const bson_t *trans_bson = transaction_find_by_oid_and_user (
				&trans->oid, &trans->user_oid,
				trans_no_user_query_opts
			);

			if (trans_bson) {
				size_t json_len = 0;
				char *json = bson_as_relaxed_extended_json (trans_bson, &json_len);
				if (json) {
					(void) http_response_json_custom_reference_send (
						http_receive, 200, json, json_len
					);

					free (json);
				}

				bson_destroy ((bson_t *) trans_bson);
			}

			else {
				(void) http_response_send (no_user_trans, http_receive);
			}

			pocket_trans_delete (trans);
		}
	}

	else {
		(void) http_response_send (bad_user, http_receive);
	}

}

static u8 pocket_transaction_update_handler_internal (
	Transaction *trans, const String *request_body
) {

	u8 retval = 1;

	if (request_body) {
		const char *title = NULL;
		double amount = 0;
		const char *category_id = NULL;
		const char *date = NULL;

		json_error_t error =  { 0 };
		json_t *json_body = json_loads (request_body->str, 0, &error);
		if (json_body) {
			pocket_trans_parse_json (
				json_body,
				&title, &amount,
				&category_id, &date
			);

			if (title) (void) strncpy (trans->title, title, TRANSACTION_TITLE_LEN);
			trans->amount = amount;

			json_decref (json_body);

			retval = 0;
		}

		else {
			cerver_log_error (
				"json_loads () - json error on line %d: %s\n", 
				error.line, error.text
			);
		}
	}

	return retval;

}

// POST /api/pocket/transactions/:id
// a user wants to update an existing transaction
void pocket_transaction_update_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		bson_oid_init_from_string (&user->oid, user->id);

		Transaction *trans = pocket_trans_get_by_id_and_user (
			request->params[0], &user->oid
		);

		if (trans) {
			// get update values
			if (!pocket_transaction_update_handler_internal (
				trans, request->body
			)) {
				// update the transaction in the db
				if (!mongo_update_one (
					transactions_collection,
					transaction_query_oid (&trans->oid),
					transaction_update_bson (trans)
				)) {
					(void) http_response_send (oki_doki, http_receive);
				}

				else {
					(void) http_response_send (server_error, http_receive);
				}
			}

			pocket_trans_delete (trans);
		}

		else {
			(void) http_response_send (bad_request, http_receive);
		}
	}

	else {
		(void) http_response_send (bad_user, http_receive);
	}

}

// DELETE /api/pocket/transactions/:id
// deletes an existing user's transaction
void pocket_transaction_delete_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	const String *trans_id = request->params[0];

	User *user = (User *) request->decoded_data;
	if (user) {
		bson_t *trans_query = bson_new ();
		if (trans_query) {
			bson_oid_t oid = { 0 };

			bson_oid_init_from_string (&oid, trans_id->str);
			(void) bson_append_oid (trans_query, "_id", -1, &oid);

			bson_oid_init_from_string (&oid, user->id);
			(void) bson_append_oid (trans_query, "user", -1, &oid);

			if (!mongo_delete_one (transactions_collection, trans_query)) {
				#ifdef POCKET_DEBUG
				cerver_log_debug ("Deleted transaction %s", trans_id->str);
				#endif

				(void) http_response_send (trans_deleted_success, http_receive);
			}

			else {
				(void) http_response_send (trans_deleted_bad, http_receive);
			}
		}

		else {
			(void) http_response_send (server_error, http_receive);
		}
	}

	else {
		(void) http_response_send (bad_user, http_receive);
	}

}