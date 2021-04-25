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

#include "pocket.h"

#include "controllers/places.h"
#include "controllers/users.h"

#include "models/place.h"
#include "models/user.h"

// GET /api/pocket/places
// get all the authenticated user's places
void pocket_places_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		size_t json_len = 0;
		char *json = places_get_all_by_user_to_json (
			&user->oid, place_no_user_query_opts,
			&json_len
		);

		if (json) {
			(void) http_response_json_custom_reference_send (
				http_receive,
				HTTP_STATUS_OK,
				json, json_len
			);

			free (json);
		}

		else {
			(void) http_response_send (no_user_places, http_receive);
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
	}

}

// POST /api/pocket/places
// a user has requested to create a new place
void pocket_place_create_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		PocketError error = pocket_place_create (
			user, request->body
		);

		switch (error) {
			case POCKET_ERROR_NONE: {
				// return success to user
				(void) http_response_send (
					place_created_success,
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

// GET /api/pocket/places/:id
// returns information about an existing place that belongs to a user
void pocket_place_get_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	const String *place_id = request->params[0];

	User *user = (User *) request->decoded_data;
	if (user) {
		if (place_id) {
			size_t json_len = 0;
			char *json = NULL;

			if (!pocket_place_get_by_id_and_user_to_json (
				place_id->str, &user->oid,
				place_no_user_query_opts,
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
				(void) http_response_send (no_user_place, http_receive);
			}
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
	}

}

// PUT /api/pocket/places/:id
// a user wants to update an existing place
void pocket_place_update_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		PocketError error = pocket_place_update (
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

// DELETE /api/pocket/places/:id
// deletes an existing user's place
void pocket_place_delete_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	const String *place_id = request->params[0];

	User *user = (User *) request->decoded_data;
	if (user) {
		switch (pocket_place_delete (user, place_id)) {
			case POCKET_ERROR_NONE:
				(void) http_response_send (place_deleted_success, http_receive);
				break;

			default:
				(void) http_response_send (place_deleted_bad, http_receive);
				break;
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
	}

}