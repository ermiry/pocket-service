#include <stdlib.h>

#include <time.h>

#include <cerver/types/string.h>

#include <cerver/collections/pool.h>

#include <cerver/http/response.h>
#include <cerver/http/json/json.h>

#include <cerver/utils/log.h>

#include <cmongo/crud.h>
#include <cmongo/select.h>

#include "errors.h"

#include "models/place.h"
#include "models/user.h"

#include "controllers/places.h"

Pool *places_pool = NULL;

const bson_t *place_no_user_query_opts = NULL;
static CMongoSelect *place_no_user_select = NULL;

HttpResponse *no_user_places = NULL;
HttpResponse *no_user_place = NULL;

HttpResponse *place_created_success = NULL;
HttpResponse *place_created_bad = NULL;
HttpResponse *place_deleted_success = NULL;
HttpResponse *place_deleted_bad = NULL;

void pocket_place_return (void *place_ptr);

static unsigned int pocket_places_init_pool (void) {

	unsigned int retval = 1;

	places_pool = pool_create (place_delete);
	if (places_pool) {
		pool_set_create (places_pool, place_new);
		pool_set_produce_if_empty (places_pool, true);
		if (!pool_init (places_pool, place_new, DEFAULT_PLACES_POOL_INIT)) {
			retval = 0;
		}

		else {
			cerver_log_error ("Failed to init places pool!");
		}
	}

	else {
		cerver_log_error ("Failed to create places pool!");
	}

	return retval;

}

static unsigned int pocket_places_init_query_opts (void) {

	unsigned int retval = 1;

	place_no_user_select = cmongo_select_new ();
	(void) cmongo_select_insert_field (place_no_user_select, "name");
	(void) cmongo_select_insert_field (place_no_user_select, "description");
	(void) cmongo_select_insert_field (place_no_user_select, "type");
	(void) cmongo_select_insert_field (place_no_user_select, "location");
	(void) cmongo_select_insert_field (place_no_user_select, "site");
	(void) cmongo_select_insert_field (place_no_user_select, "color");
	(void) cmongo_select_insert_field (place_no_user_select, "date");

	place_no_user_query_opts = mongo_find_generate_opts (place_no_user_select);

	if (place_no_user_query_opts) retval = 0;

	return retval;

}

static unsigned int pocket_places_init_responses (void) {

	unsigned int retval = 1;

	no_user_places = http_response_json_key_value (
		HTTP_STATUS_NOT_FOUND, "msg", "No user's places"
	);

	no_user_place = http_response_json_key_value (
		HTTP_STATUS_NOT_FOUND, "msg", "User's place was not found"
	);

	place_created_success = http_response_json_key_value (
		HTTP_STATUS_OK, "oki", "doki"
	);

	place_created_bad = http_response_json_key_value (
		HTTP_STATUS_BAD_REQUEST, "error", "Failed to create place!"
	);

	place_deleted_success = http_response_json_key_value (
		HTTP_STATUS_OK, "oki", "doki"
	);

	place_deleted_bad = http_response_json_key_value (
		HTTP_STATUS_BAD_REQUEST, "error", "Failed to delete place!"
	);

	if (
		no_user_places && no_user_place
		&& place_created_success && place_created_bad
		&& place_deleted_success && place_deleted_bad
	) retval = 0;

	return retval;

}

unsigned int pocket_places_init (void) {

	unsigned int errors = 0;

	errors |= pocket_places_init_pool ();

	errors |= pocket_places_init_query_opts ();

	errors |= pocket_places_init_responses ();

	return errors;

}

void pocket_places_end (void) {

	cmongo_select_delete (place_no_user_select);
	bson_destroy ((bson_t *) place_no_user_query_opts);

	pool_delete (places_pool);
	places_pool = NULL;

	http_response_delete (no_user_places);
	http_response_delete (no_user_place);

	http_response_delete (place_created_success);
	http_response_delete (place_created_bad);
	http_response_delete (place_deleted_success);
	http_response_delete (place_deleted_bad);

}

Place *pocket_place_get_by_id_and_user (
	const String *place_id, const bson_oid_t *user_oid
) {

	Place *place = NULL;

	if (place_id) {
		place = (Place *) pool_pop (places_pool);
		if (place) {
			bson_oid_init_from_string (&place->oid, place_id->str);

			if (place_get_by_oid_and_user (
				place,
				&place->oid, user_oid,
				NULL
			)) {
				pocket_place_return (place);
				place = NULL;
			}
		}
	}

	return place;

}

u8 pocket_place_get_by_id_and_user_to_json (
	const char *place_id, const bson_oid_t *user_oid,
	const bson_t *query_opts,
	char **json, size_t *json_len
) {

	u8 retval = 1;

	if (place_id) {
		bson_oid_t place_oid = { 0 };
		bson_oid_init_from_string (&place_oid, place_id);

		retval = place_get_by_oid_and_user_to_json (
			&place_oid, user_oid,
			query_opts,
			json, json_len
		);
	}

	return retval;

}

static Place *pocket_place_create_actual (
	const char *user_id,
	const char *name, const char *description,
	const char *type,
	const char *link,
	const char *logo,
	const char *color
) {

	Place *place = (Place *) pool_pop (places_pool);
	if (place) {
		bson_oid_init (&place->oid, NULL);

		bson_oid_init_from_string (&place->user_oid, user_id);

		if (name) (void) strncpy (place->name, name, PLACE_NAME_SIZE - 1);
		if (description) (void) strncpy (place->description, description, PLACE_DESCRIPTION_SIZE - 1);
		
		place->type = place_type_from_value_string (type);

		switch (place->type) {
			case PLACE_TYPE_NONE: break;

			case PLACE_TYPE_LOCATION: break;

			case PLACE_TYPE_SITE: {
				if (link) (void) strncpy (place->site.link, link, SITE_LINK_SIZE - 1);
				if (logo) (void) strncpy (place->site.logo, logo, SITE_LOGO_SIZE - 1);
			} break;
			
			default: break;
		}

		if (color) (void) strncpy (place->color, color, PLACE_COLOR_SIZE - 1);

		place->date = time (NULL);
	}

	return place;

}

static void pocket_place_parse_json (
	json_t *json_body,
	const char **name,
	const char **description,
	const char **type,
	const char **link,
	const char **logo,
	const char **color
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

			else if (!strcmp (key, "type")) {
				*type = json_string_value (value);
				#ifdef POCKET_DEBUG
				(void) printf ("type: \"%s\"\n", *type);
				#endif
			}

			else if (!strcmp (key, "link")) {
				*link = json_string_value (value);
				#ifdef POCKET_DEBUG
				(void) printf ("link: \"%s\"\n", *link);
				#endif
			}

			else if (!strcmp (key, "logo")) {
				*logo = json_string_value (value);
				#ifdef POCKET_DEBUG
				(void) printf ("logo: \"%s\"\n", *logo);
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

static PocketError pocket_place_create_parse_json (
	Place **place,
	const char *user_id, const String *request_body
) {

	PocketError error = POCKET_ERROR_NONE;

	const char *name = NULL;
	const char *description = NULL;
	const char *type = NULL;
	const char *link = NULL;
	const char *logo = NULL;
	const char *color = NULL;

	json_error_t json_error =  { 0 };
	json_t *json_body = json_loads (request_body->str, 0, &json_error);
	if (json_body) {
		pocket_place_parse_json (
			json_body,
			&name,
			&description,
			&type,
			&link,
			&logo,
			&color
		);

		place = pocket_place_create_actual (
			user_id,
			name, description,
			type,
			link,
			logo,
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

PocketError pocket_place_create (
	const User *user, const String *request_body
) {

	PocketError error = POCKET_ERROR_NONE;

	if (request_body) {
		Place *place = NULL;

		error = pocket_place_create_parse_json (
			&place,
			user->id, request_body
		);

		if (error == POCKET_ERROR_NONE) {
			#ifdef POCKET_DEBUG
			place_print (place);
			#endif

			if (!place_insert_one (place)) {
				// update users values
				(void) user_add_place (user);
			}

			else {
				error = POCKET_ERROR_SERVER_ERROR;
			}

			pocket_place_return (place);
		}
	}

	else {
		#ifdef POCKET_DEBUG
		cerver_log_error ("Missing request body to create place!");
		#endif

		error = POCKET_ERROR_BAD_REQUEST;
	}

	return error;

}

void pocket_place_return (void *place_ptr) {

	(void) memset (place_ptr, 0, sizeof (Place));
	(void) pool_push (places_pool, place_ptr);

}