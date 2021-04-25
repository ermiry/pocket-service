#include <stdlib.h>

#include <time.h>

#include <cerver/types/string.h>

#include <cerver/http/response.h>
#include <cerver/http/json/json.h>

#include <cerver/collections/pool.h>

#include <cerver/utils/log.h>

#include <cmongo/crud.h>
#include <cmongo/select.h>

#include "errors.h"

#include "models/category.h"
#include "models/user.h"

#include "controllers/categories.h"

Pool *categories_pool = NULL;

const bson_t *category_no_user_query_opts = NULL;
static CMongoSelect *category_no_user_select = NULL;

HttpResponse *no_user_categories = NULL;
HttpResponse *no_user_category = NULL;

HttpResponse *category_created_success = NULL;
HttpResponse *category_created_bad = NULL;
HttpResponse *category_deleted_success = NULL;
HttpResponse *category_deleted_bad = NULL;

void pocket_category_return (void *category_ptr);

static unsigned int pocket_categories_init_pool (void) {

	unsigned int retval = 1;

	categories_pool = pool_create (category_delete);
	if (categories_pool) {
		pool_set_create (categories_pool, category_new);
		pool_set_produce_if_empty (categories_pool, true);
		if (!pool_init (categories_pool, category_new, DEFAULT_CATEGORIES_POOL_INIT)) {
			retval = 0;
		}

		else {
			cerver_log_error ("Failed to init categories pool!");
		}
	}

	else {
		cerver_log_error ("Failed to create categories pool!");
	}

	return retval;

}

static unsigned int pocket_categories_init_query_opts (void) {

	unsigned int retval = 1;

	category_no_user_select = cmongo_select_new ();
	(void) cmongo_select_insert_field (category_no_user_select, "title");
	(void) cmongo_select_insert_field (category_no_user_select, "description");
	(void) cmongo_select_insert_field (category_no_user_select, "color");
	(void) cmongo_select_insert_field (category_no_user_select, "date");

	category_no_user_query_opts = mongo_find_generate_opts (category_no_user_select);

	if (category_no_user_query_opts) retval = 0;

	return retval;

}

static unsigned int pocket_categories_init_responses (void) {

	unsigned int retval = 1;

	no_user_categories = http_response_json_key_value (
		HTTP_STATUS_NOT_FOUND, "msg", "Failed to get user's categories"
	);

	no_user_category = http_response_json_key_value (
		HTTP_STATUS_NOT_FOUND, "msg", "User's category was not found"
	);

	category_created_success = http_response_json_key_value (
		HTTP_STATUS_OK, "oki", "doki"
	);

	category_created_bad = http_response_json_key_value (
		HTTP_STATUS_BAD_REQUEST, "error", "Failed to create category!"
	);

	category_deleted_success = http_response_json_key_value (
		HTTP_STATUS_OK, "oki", "doki"
	);

	category_deleted_bad = http_response_json_key_value (
		HTTP_STATUS_BAD_REQUEST, "error", "Failed to delete category!"
	);

	if (
		no_user_categories && no_user_category
		&& category_created_success && category_created_bad
		&& category_deleted_success && category_deleted_bad
	) retval = 0;

	return retval;

}

unsigned int pocket_categories_init (void) {

	unsigned int errors = 0;

	errors |= pocket_categories_init_pool ();

	errors |= pocket_categories_init_query_opts ();

	errors |= pocket_categories_init_responses ();

	return errors;

}

void pocket_categories_end (void) {

	cmongo_select_delete (category_no_user_select);
	bson_destroy ((bson_t *) category_no_user_query_opts);

	pool_delete (categories_pool);
	categories_pool = NULL;

	http_response_delete (no_user_categories);
	http_response_delete (no_user_category);

	http_response_delete (category_created_success);
	http_response_delete (category_created_bad);
	http_response_delete (category_deleted_success);
	http_response_delete (category_deleted_bad);

}

Category *pocket_category_get_by_id_and_user (
	const String *category_id, const bson_oid_t *user_oid
) {

	Category *category = NULL;

	if (category_id) {
		category = (Category *) pool_pop (categories_pool);
		if (category) {
			bson_oid_init_from_string (&category->oid, category_id->str);

			if (category_get_by_oid_and_user (
				category,
				&category->oid, user_oid,
				NULL
			)) {
				pocket_category_return (category);
				category = NULL;
			}
		}
	}

	return category;

}

u8 pocket_category_get_by_id_and_user_to_json (
	const char *category_id, const bson_oid_t *user_oid,
	const bson_t *query_opts,
	char **json, size_t *json_len
) {

	u8 retval = 1;

	if (category_id) {
		bson_oid_t category_oid = { 0 };
		bson_oid_init_from_string (&category_oid, category_id);

		retval = category_get_by_oid_and_user_to_json (
			&category_oid, user_oid,
			query_opts,
			json, json_len
		);
	}

	return retval;

}

static Category *pocket_category_create_actual (
	const char *user_id,
	const char *title, const char *description,
	const char *color
) {

	Category *category = (Category *) pool_pop (categories_pool);
	if (category) {
		bson_oid_init (&category->oid, NULL);

		bson_oid_init_from_string (&category->user_oid, user_id);

		if (title) (void) strncpy (category->title, title, CATEGORY_TITLE_SIZE - 1);
		if (description) (void) strncpy (category->description, description, CATEGORY_DESCRIPTION_SIZE - 1);

		if (color) (void) strncpy (category->color, color, CATEGORY_COLOR_SIZE - 1);
		
		category->date = time (NULL);
	}

	return category;

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

static PocketError pocket_category_create_parse_json (
	Category **category,
	const char *user_id, const String *request_body
) {

	PocketError error = POCKET_ERROR_NONE;

	const char *title = NULL;
	const char *description = NULL;
	const char *color = NULL;

	json_error_t json_error =  { 0 };
	json_t *json_body = json_loads (request_body->str, 0, &json_error);
	if (json_body) {
		pocket_category_parse_json (
			json_body,
			&title,
			&description,
			&color
		);

		*category = pocket_category_create_actual (
			user_id,
			title, description,
			color
		);

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

PocketError pocket_category_create (
	const User *user, const String *request_body
) {

	PocketError error = POCKET_ERROR_NONE;

	if (request_body) {
		Category *category = NULL;

		error = pocket_category_create_parse_json (
			&category,
			user->id, request_body
		);

		if (error == POCKET_ERROR_NONE) {
			#ifdef POCKET_DEBUG
			category_print (category);
			#endif

			if (!category_insert_one (
				category
			)) {
				// update users values
				(void) user_add_category (user);
			}

			else {
				error = POCKET_ERROR_SERVER_ERROR;
			}
			
			pocket_category_return (category);
		}
	}

	else {
		#ifdef POCKET_DEBUG
		cerver_log_error ("Missing request body to create category!");
		#endif

		error = POCKET_ERROR_BAD_REQUEST;
	}

	return error;

}

static PocketError pocket_category_update_parse_json (
	Category *category, const String *request_body
) {

	PocketError error = POCKET_ERROR_NONE;

	const char *title = NULL;
	const char *description = NULL;
	const char *color = NULL;

	json_error_t json_error =  { 0 };
	json_t *json_body = json_loads (request_body->str, 0, &json_error);
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

PocketError pocket_category_update (
	const User *user, const String *category_id,
	const String *request_body
) {

	PocketError error = POCKET_ERROR_NONE;

	if (request_body) {
		Category *category = pocket_category_get_by_id_and_user (
			category_id, &user->oid
		);

		if (category) {
			// get update values
			if (pocket_category_update_parse_json (
				category, request_body
			) == POCKET_ERROR_NONE) {
				// update the category in the db
				if (category_update_one (category)) {
					error = POCKET_ERROR_SERVER_ERROR;
				}
			}

			pocket_category_return (category);
		}

		else {
			#ifdef POCKET_DEBUG
			cerver_log_error ("Failed to get matching category!");
			#endif

			error = POCKET_ERROR_NOT_FOUND;
		}
	}

	else {
		#ifdef POCKET_DEBUG
		cerver_log_error ("Missing request body to update category!");
		#endif

		error = POCKET_ERROR_BAD_REQUEST;
	}

	return error;

}

// TODO: handle things that reference the requested category
PocketError pocket_category_delete (
	const User *user, const String *category_id
) {

	PocketError error = POCKET_ERROR_NONE;

	bson_oid_t oid = { 0 };
	bson_oid_init_from_string (&oid, category_id->str);

	if (!category_delete_one_by_oid_and_user (
		&oid, &user->oid
	)) {
		#ifdef POCKET_DEBUG
		cerver_log_debug ("Deleted category %s", category_id->str);
		#endif
	}

	else {
		error = POCKET_ERROR_BAD_REQUEST;
	}

	return error;

}

void pocket_category_return (void *category_ptr) {

	(void) memset (category_ptr, 0, sizeof (Category));
	(void) pool_push (categories_pool, category_ptr);

}