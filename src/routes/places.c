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

// POST /api/pocket/places
// a user has requested to create a new place
void pocket_place_create_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

}

// GET /api/pocket/places/:id
// returns information about an existing place that belongs to a user
void pocket_place_get_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

}

// PUT /api/pocket/places/:id
// a user wants to update an existing place
void pocket_place_update_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

}

// DELETE /api/pocket/places/:id
// deletes an existing user's place
void pocket_place_delete_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

}