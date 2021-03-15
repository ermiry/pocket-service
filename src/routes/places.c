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
				200,
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

static void pocket_place_parse_json (
	json_t *json_body,
	const char **name,
	const char **description
) {

	// get values from json to create a new place
	const char *key = NULL;
	json_t *value = NULL;
	if (json_typeof (json_body) == JSON_OBJECT) {
		json_object_foreach (json_body, key, value) {
			if (!strcmp (key, "name")) {
				*name = json_string_value (value);
				#ifdef POCKET_DEBUG
				(void) printf ("name: \"%s\"\n", *name);
				#endif
			}

			else if (!strcmp (key, "description")) {
				*description = json_string_value (value);
				#ifdef POCKET_DEBUG
				(void) printf ("description: \"%s\"\n", *description);
				#endif
			}
		}
	}

}

static Place *pocket_place_create_handler_internal (
	const char *user_id, const String *request_body
) {

	Place *place = NULL;

	if (request_body) {
		const char *name = NULL;
		const char *description = NULL;

		json_error_t error =  { 0 };
		json_t *json_body = json_loads (request_body->str, 0, &error);
		if (json_body) {
			pocket_place_parse_json (
				json_body,
				&name,
				&description
			);

			place = pocket_place_create (
				user_id,
				name, description
			);

			json_decref (json_body);
		}

		else {
			cerver_log_error (
				"json_loads () - json error on line %d: %s\n", 
				error.line, error.text
			);
		}
	}

	else {
		cerver_log_error ("Missing request body to create place!");
	}

	return place;

}

// POST /api/pocket/places
// a user has requested to create a new place
void pocket_place_create_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		Place *place = pocket_place_create_handler_internal (
			user->id, request->body
		);

		if (place) {
			#ifdef POCKET_DEBUG
			place_print (place);
			#endif

			if (!place_insert_one (place)) {
				// update users values
				(void) user_add_place (user);

				// return success to user
				(void) http_response_send (
					place_created_success,
					http_receive
				);
			}
		}

		else {
			(void) http_response_send (
				place_created_bad,
				http_receive
			);
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
						http_receive, 200, json, json_len
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

static u8 pocket_place_update_handler_internal (
	Place *place, const String *request_body
) {

	u8 retval = 1;

	if (request_body) {
		const char *title = NULL;
		const char *description = NULL;

		json_error_t error =  { 0 };
		json_t *json_body = json_loads (request_body->str, 0, &error);
		if (json_body) {
			pocket_place_parse_json (
				json_body,
				&title,
				&description
			);

			if (title) (void) strncpy (place->name, title, PLACE_NAME_LEN - 1);
			if (description) (void) strncpy (place->description, description, PLACE_DESCRIPTION_LEN - 1);

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
		cerver_log_error ("Missing request body to update place!");
	}

	return retval;

}

// PUT /api/pocket/places/:id
// a user wants to update an existing place
void pocket_place_update_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		bson_oid_init_from_string (&user->oid, user->id);

		Place *place = pocket_place_get_by_id_and_user (
			request->params[0], &user->oid
		);

		if (place) {
			// get update values
			if (!pocket_place_update_handler_internal (
				place, request->body
			)) {
				if (!place_update_one (place)) {
					(void) http_response_send (oki_doki, http_receive);
				}

				else {
					(void) http_response_send (server_error, http_receive);
				}
			}

			else {
				(void) http_response_send (bad_request_error, http_receive);
			}

			pocket_place_delete (place);
		}

		else {
			(void) http_response_send (bad_request_error, http_receive);
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
	}

}

// TODO: handle things that reference the requested place
// DELETE /api/pocket/places/:id
// deletes an existing user's place
void pocket_place_delete_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	const String *place_id = request->params[0];

	User *user = (User *) request->decoded_data;
	if (user) {
		bson_oid_t oid = { 0 };
		bson_oid_init_from_string (&oid, place_id->str);

		if (!place_delete_one_by_oid_and_user (
			&oid, &user->oid
		)) {
			#ifdef POCKET_DEBUG
			cerver_log_debug ("Deleted place %s", place_id->str);
			#endif

			(void) http_response_send (place_deleted_success, http_receive);
		}

		else {
			(void) http_response_send (place_deleted_bad, http_receive);
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
	}

}