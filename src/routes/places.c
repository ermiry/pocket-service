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

#include "mongo.h"
#include "pocket.h"

#include "controllers/places.h"
#include "controllers/users.h"

#include "models/place.h"
#include "models/user.h"

static char *pocket_places_handler_generate_json (
	User *user,
	mongoc_cursor_t *places_cursor,
	size_t *json_len
) {

	char *retval = NULL;

	bson_t *doc = bson_new ();
	if (doc) {
		(void) bson_append_int32 (doc, "count", -1, user->places_count);

		bson_t places_array = { 0 };
		(void) bson_append_array_begin (doc, "places", -1, &places_array);
		char buf[16] = { 0 };
		const char *key = NULL;
		size_t keylen = 0;

		int i = 0;
		const bson_t *place_doc = NULL;
		while (mongoc_cursor_next (places_cursor, &place_doc)) {
			keylen = bson_uint32_to_string (i, &key, buf, sizeof (buf));
			(void) bson_append_document (&places_array, key, (int) keylen, place_doc);

			bson_destroy ((bson_t *) place_doc);

			i++;
		}
		(void) bson_append_array_end (doc, &places_array);

		retval = bson_as_relaxed_extended_json (doc, json_len);
	}

	return retval;

}

// GET /api/pocket/places
// get all the authenticated user's places
void pocket_places_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		// TODO: query opts
		// get user's places from the db
		if (!user_get_by_id (user, user->id, NULL)) {
			mongoc_cursor_t *places_cursor = places_get_all_by_user (
				&user->oid, NULL
			);

			if (places_cursor) {
				// convert them to json and send them back
				size_t json_len = 0;
				char *json = pocket_places_handler_generate_json (
					user, places_cursor, &json_len
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

				mongoc_cursor_destroy (places_cursor);
			}

			else {
				(void) http_response_send (no_user_places, http_receive);
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
				(void) printf ("name: \"%s\"\n", *name);
			}

			else if (!strcmp (key, "description")) {
				*description = json_string_value (value);
				(void) printf ("description: \"%s\"\n", *description);
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

			if (!mongo_insert_one (
				places_collection,
				place_to_bson (place)
			)) {
				// update users values
				(void) mongo_update_one (
					users_collection,
					user_query_id (user->id),
					user_create_update_pocket_places ()
				);

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
		(void) http_response_send (bad_user, http_receive);
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
		Place *place = (Place *) pool_pop (places_pool);
		if (place) {
			bson_oid_init_from_string (&place->oid, place_id->str);
			bson_oid_init_from_string (&place->user_oid, user->id);

			const bson_t *place_bson = place_find_by_oid_and_user (
				&place->oid, &place->user_oid,
				place_no_user_query_opts
			);

			if (place_bson) {
				size_t json_len = 0;
				char *json = bson_as_relaxed_extended_json (place_bson, &json_len);
				if (json) {
					(void) http_response_json_custom_reference_send (
						http_receive, 200, json, json_len
					);

					free (json);
				}

				bson_destroy ((bson_t *) place_bson);
			}

			else {
				(void) http_response_send (no_user_place, http_receive);
			}

			pocket_place_delete (place);
		}

		else {
			(void) http_response_send (server_error, http_receive);
		}
	}

	else {
		(void) http_response_send (bad_user, http_receive);
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

	}

	else {
		(void) http_response_send (bad_user, http_receive);
	}

}

// DELETE /api/pocket/places/:id
// deletes an existing user's place
void pocket_place_delete_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {

	}

	else {
		(void) http_response_send (bad_user, http_receive);
	}

}