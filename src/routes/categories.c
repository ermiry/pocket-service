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

#include "controllers/categories.h"
#include "controllers/users.h"

#include "models/category.h"
#include "models/user.h"

// GET /api/pocket/categories
// get all the authenticated user's categories
void pocket_categories_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		size_t json_len = 0;
		char *json = categories_get_all_by_user_to_json (
			&user->oid, category_no_user_query_opts,
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
			(void) http_response_send (no_user_categories, http_receive);
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
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
				#ifdef POCKET_DEBUG
				(void) printf ("title: \"%s\"\n", *title);
				#endif
			}

			else if (!strcmp (key, "description")) {
				*description = json_string_value (value);
				#ifdef POCKET_DEBUG
				(void) printf ("description: \"%s\"\n", *description);
				#endif
			}

			else if (!strcmp (key, "color")) {
				*color = json_string_value (value);
				#ifdef POCKET_DEBUG
				(void) printf ("color: \"%s\"\n", *color);
				#endif
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

			if (!category_insert_one (
				category
			)) {
				// update users values
				(void) user_add_category (user);

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
		(void) http_response_send (bad_user_error, http_receive);
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
		if (category_id) {
			size_t json_len = 0;
			char *json = NULL;

			if (!pocket_category_get_by_id_and_user_to_json (
				category_id->str, &user->oid,
				category_no_user_query_opts,
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
				(void) http_response_send (no_user_category, http_receive);
			}
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
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

			if (title) (void) strncpy (category->title, title, CATEGORY_TITLE_SIZE - 1);
			if (description) (void) strncpy (category->description, description, CATEGORY_DESCRIPTION_SIZE - 1);
			if (color) (void) strncpy (category->color, color, CATEGORY_COLOR_SIZE - 1);

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
				if (!category_update_one (category)) {
					(void) http_response_send (oki_doki, http_receive);
				}

				else {
					(void) http_response_send (server_error, http_receive);
				}
			}

			else {
				(void) http_response_send (bad_request_error, http_receive);
			}

			pocket_category_delete (category);
		}

		else {
			(void) http_response_send (bad_request_error, http_receive);
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
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
		bson_oid_t oid = { 0 };
		bson_oid_init_from_string (&oid, category_id->str);

		if (!category_delete_one_by_oid_and_user (
			&oid, &user->oid
		)) {
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
		(void) http_response_send (bad_user_error, http_receive);
	}

}