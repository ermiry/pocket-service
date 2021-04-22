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
#include "pocket.h"

#include "controllers/categories.h"
#include "controllers/transactions.h"
#include "controllers/users.h"

#include "models/category.h"
#include "models/user.h"

// GET /api/pocket/transactions
// get all the authenticated user's transactions
void pocket_transactions_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		size_t json_len = 0;
		char *json = transactions_get_all_by_user_to_json (
			&user->oid, trans_no_user_query_opts,
			&json_len
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
			(void) http_response_send (no_user_trans, http_receive);
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
	}

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

static PocketError pocket_transaction_create_handler_internal (
	Transaction **trans,
	const char *user_id, const String *request_body
) {

	PocketError error = POCKET_ERROR_NONE;

	if (request_body) {
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
				*trans = pocket_trans_create (
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
	}

	else {
		cerver_log_error ("Missing request body to create trans!");

		error = POCKET_ERROR_BAD_REQUEST;
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

			if (!transaction_insert_one (trans)) {
				// update users values
				(void) user_add_transactions (user);

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
		(void) http_response_send (bad_user_error, http_receive);
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
		if (trans_id) {
			size_t json_len = 0;
			char *json = NULL;

			if (!pocket_trans_get_by_id_and_user_to_json (
				trans_id->str, &user->oid,
				trans_no_user_query_opts,
				&json, &json_len
			)) {
				if (json) {
					(void) http_response_json_custom_reference_send (
						http_receive, 200, json, json_len
					);

					free (json);
				}

				else {
					(void) http_response_send (server_error, http_receive);
				}
			}

			else {
				(void) http_response_send (no_user_trans, http_receive);
			}
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
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
		const char *place_id = NULL;
		const char *date = NULL;

		json_error_t error =  { 0 };
		json_t *json_body = json_loads (request_body->str, 0, &error);
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

			retval = 0;
		}

		else {
			cerver_log_error (
				"json_loads () - json error on line %d: %s\n",
				error.line, error.text
			);
		}
	}

	else {
		cerver_log_error ("Missing request body to update trans!");
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
				if (!transaction_update_one (trans)) {
					(void) http_response_send (oki_doki, http_receive);
				}

				else {
					(void) http_response_send (server_error, http_receive);
				}
			}

			pocket_trans_delete (trans);
		}

		else {
			(void) http_response_send (bad_request_error, http_receive);
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
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
		bson_oid_t oid = { 0 };
		bson_oid_init_from_string (&oid, trans_id->str);

		if (!transaction_delete_one_by_oid_and_user (
			&oid, &user->oid
		)) {
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
		(void) http_response_send (bad_user_error, http_receive);
	}

}