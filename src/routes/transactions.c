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
		char *json = NULL;

		if (!pocket_trans_get_all_by_user (
			&user->oid,
			&json, &json_len
		)) {
			if (json) {
				(void) http_response_json_custom_reference_send (
					http_receive,
					HTTP_STATUS_OK,
					json, json_len
				);

				free (json);
			}

			else {
				(void) http_response_send (no_user_trans, http_receive);
			}
		}

		else {
			(void) http_response_send (no_user_trans, http_receive);
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
	}

}

// POST /api/pocket/transactions
// a user has requested to create a new transaction
void pocket_transaction_create_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		PocketError error = pocket_trans_create (
			user, request->body
		);

		switch (error) {
			case POCKET_ERROR_NONE: {
				// return success to user
				(void) http_response_send (
					trans_created_success,
					http_receive
				);
			} break;

			default: {
				pocket_error_send_response (error, http_receive);
			} break;
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
						http_receive, HTTP_STATUS_OK, json, json_len
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

// POST /api/pocket/transactions/:id
// a user wants to update an existing transaction
void pocket_transaction_update_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		PocketError error = pocket_trans_update (
			user, request->params[0], request->body
		);

		switch (error) {
			case POCKET_ERROR_NONE: {
				(void) http_response_send (oki_doki, http_receive);
			} break;

			default: {
				pocket_error_send_response (error, http_receive);
			} break;
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
		switch (pocket_trans_delete (user, trans_id)) {
			case POCKET_ERROR_NONE:
				(void) http_response_send (trans_deleted_success, http_receive);
				break;

			default:
				(void) http_response_send (trans_deleted_bad, http_receive);
				break;
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
	}

}