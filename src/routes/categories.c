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

#include "controllers/categories.h"
#include "controllers/users.h"

#include "models/category.h"
#include "models/user.h"

static char *pocket_categories_handler_generate_json (
	User *user,
	mongoc_cursor_t *categories_cursor,
	size_t *json_len
) {

	char *retval = NULL;

	bson_t *doc = bson_new ();
	if (doc) {
		(void) bson_append_int32 (doc, "count", -1, user->categories_count);

		bson_t categories_array = { 0 };
		(void) bson_append_array_begin (doc, "categories", -1, &categories_array);
		char buf[16] = { 0 };
		const char *key = NULL;
		size_t keylen = 0;

		int i = 0;
		const bson_t *category_doc = NULL;
		while (mongoc_cursor_next (categories_cursor, &category_doc)) {
			keylen = bson_uint32_to_string (i, &key, buf, sizeof (buf));
			(void) bson_append_document (&categories_array, key, (int) keylen, category_doc);

			bson_destroy ((bson_t *) category_doc);

			i++;
		}
		(void) bson_append_array_end (doc, &categories_array);

		retval = bson_as_relaxed_extended_json (doc, json_len);
	}

	return retval;

}

// GET /api/pocket/categories
// get all the authenticated user's categories
void pocket_categories_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		// get user's categories from the db
		if (!user_get_by_id (user, user->id, user_categories_query_opts)) {
			mongoc_cursor_t *categories_cursor = categories_get_all_by_user (
				&user->oid, category_no_user_query_opts
			);

			if (categories_cursor) {
				// convert them to json and send them back
				size_t json_len = 0;
				char *json = pocket_categories_handler_generate_json (
					user, categories_cursor, &json_len
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

				mongoc_cursor_destroy (categories_cursor);
			}

			else {
				(void) http_response_send (no_user_categories, http_receive);
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

static void pocket_category_parse_json (
	json_t *json_body,
	const char **title,
	const char **description,
	const char **color
) {

	// get values from json to create a new category
	const char *key = NULL;
	json_t *value = NULL;
	if (json_typeof (json_body) == JSON_OBJECT) {
		json_object_foreach (json_body, key, value) {
			if (!strcmp (key, "title")) {
				*title = json_string_value (value);
				(void) printf ("title: \"%s\"\n", *title);
			}

			else if (!strcmp (key, "description")) {
				*description = json_string_value (value);
				(void) printf ("description: \"%s\"\n", *description);
			}

			else if (!strcmp (key, "color")) {
				*color = json_string_value (value);
				(void) printf ("color: \"%s\"\n", *color);
			}
		}
	}

}

static Category *pocket_category_create_handler_internal (
	const char *user_id, const String *request_body
) {

	Category *category = NULL;

	if (request_body) {
		const char *title = NULL;
		const char *description = NULL;
		const char *color = NULL;

		json_error_t error =  { 0 };
		json_t *json_body = json_loads (request_body->str, 0, &error);
		if (json_body) {
			pocket_category_parse_json (
				json_body,
				&title,
				&description,
				&color
			);

			category = pocket_category_create (
				user_id,
				title, description,
				color
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
		cerver_log_error ("Missing request body to create category!");
	}

	return category;

}

// POST /api/pocket/categories
// a user has requested to create a new category
void pocket_category_create_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		Category *category = pocket_category_create_handler_internal (
			user->id, request->body
		);

		if (category) {
			#ifdef POCKET_DEBUG
			category_print (category);
			#endif

			if (!mongo_insert_one (
				categories_collection,
				category_to_bson (category)
			)) {
				// update users values
				(void) mongo_update_one (
					users_collection,
					user_query_id (user->id),
					user_create_update_pocket_categories ()
				);

				// return success to user
				(void) http_response_send (
					category_created_success,
					http_receive
				);
			}

			else {
				(void) http_response_send (
					category_created_bad,
					http_receive
				);
			}
			
			pocket_category_delete (category);
		}

		else {
			(void) http_response_send (
				category_created_bad,
				http_receive
			);
		}
	}

	else {
		(void) http_response_send (bad_user, http_receive);
	}

}

// GET /api/pocket/categories/:id
// returns information about an existing category that belongs to a user
void pocket_category_get_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	const String *category_id = request->params[0];

	User *user = (User *) request->decoded_data;
	if (user) {
		Category *category = (Category *) pool_pop (categories_pool);
		if (category) {
			bson_oid_init_from_string (&category->oid, category_id->str);
			bson_oid_init_from_string (&category->user_oid, user->id);

			const bson_t *category_bson = category_find_by_oid_and_user (
				&category->oid, &category->user_oid,
				category_no_user_query_opts
			);

			if (category_bson) {
				size_t json_len = 0;
				char *json = bson_as_relaxed_extended_json (category_bson, &json_len);
				if (json) {
					(void) http_response_json_custom_reference_send (
						http_receive, 200, json, json_len
					);

					free (json);
				}

				bson_destroy ((bson_t *) category_bson);
			}

			else {
				(void) http_response_send (no_user_category, http_receive);
			}

			pocket_category_delete (category);
		}

		else {
			(void) http_response_send (server_error, http_receive);
		}
	}

	else {
		(void) http_response_send (bad_user, http_receive);
	}

}

static u8 pocket_category_update_handler_internal (
	Category *category, const String *request_body
) {

	u8 retval = 1;

	if (request_body) {
		const char *title = NULL;
		const char *description = NULL;
		const char *color = NULL;

		json_error_t error =  { 0 };
		json_t *json_body = json_loads (request_body->str, 0, &error);
		if (json_body) {
			pocket_category_parse_json (
				json_body,
				&title,
				&description,
				&color
			);

			if (title) (void) strncpy (category->title, title, CATEGORY_TITLE_LEN);
			if (description) (void) strncpy (category->description, description, CATEGORY_DESCRIPTION_LEN);
			if (color) (void) strncpy (category->color, color, CATEGORY_COLOR_LEN);

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
		cerver_log_error ("Missing request body to update category!");
	}

	return retval;

}

// PUT /api/pocket/categories/:id
// a user wants to update an existing category
void pocket_category_update_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		bson_oid_init_from_string (&user->oid, user->id);

		Category *category = pocket_category_get_by_id_and_user (
			request->params[0], &user->oid
		);

		if (category) {
			// get update values
			if (!pocket_category_update_handler_internal (
				category, request->body
			)) {
				// update the category in the db
				if (!mongo_update_one (
					categories_collection,
					category_query_oid (&category->oid),
					category_update_bson (category)
				)) {
					(void) http_response_send (oki_doki, http_receive);
				}

				else {
					(void) http_response_send (server_error, http_receive);
				}
			}

			else {
				(void) http_response_send (bad_request, http_receive);
			}

			pocket_category_delete (category);
		}

		else {
			(void) http_response_send (bad_request, http_receive);
		}
	}

	else {
		(void) http_response_send (bad_user, http_receive);
	}

}

// TODO: handle things that reference the requested category
// DELETE /api/pocket/categories/:id
// deletes an existing user's category
void pocket_category_delete_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	const String *category_id = request->params[0];

	User *user = (User *) request->decoded_data;
	if (user) {
		bson_t *category_query = bson_new ();
		if (category_query) {
			bson_oid_t oid = { 0 };

			bson_oid_init_from_string (&oid, category_id->str);
			(void) bson_append_oid (category_query, "_id", -1, &oid);

			bson_oid_init_from_string (&oid, user->id);
			(void) bson_append_oid (category_query, "user", -1, &oid);

			if (!mongo_delete_one (categories_collection, category_query)) {
				#ifdef POCKET_DEBUG
				cerver_log_debug ("Deleted category %s", category_id->str);
				#endif

				(void) http_response_send (category_deleted_success, http_receive);
			}

			else {
				(void) http_response_send (category_deleted_bad, http_receive);
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