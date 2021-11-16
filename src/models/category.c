#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <cerver/types/types.h>

#include <cerver/utils/log.h>

#include <cmongo/collections.h>
#include <cmongo/crud.h>
#include <cmongo/model.h>

#include "models/category.h"

static CMongoModel *categories_model = NULL;

static void category_doc_parse (
	void *category_ptr, const bson_t *category_doc
);

unsigned int categories_model_init (void) {

	unsigned int retval = 1;

	categories_model = cmongo_model_create (CATEGORIES_COLL_NAME);
	if (categories_model) {
		cmongo_model_set_parser (categories_model, category_doc_parse);

		retval = 0;
	}

	return retval;

}

void categories_model_end (void) {

	cmongo_model_delete (categories_model);

}

void *category_new (void) {

	Category *category = (Category *) malloc (sizeof (Category));
	if (category) {
		(void) memset (category, 0, sizeof (Category));
	}

	return category;

}

void category_delete (void *category_ptr) {

	if (category_ptr) free (category_ptr);

}

void category_print (const Category *category) {

	if (category) {
		(void) printf ("id: %s\n", category->id);

		(void) printf ("title: %s\n", category->title);
		(void) printf ("description: %s\n", category->description);
		(void) printf ("color: %s\n", category->color);
	}

}

static void category_doc_parse (
	void *category_ptr, const bson_t *category_doc
) {

	Category *category = (Category *) category_ptr;

	bson_iter_t iter = { 0 };
	if (bson_iter_init (&iter, category_doc)) {
		char *key = NULL;
		bson_value_t *value = NULL;
		while (bson_iter_next (&iter)) {
			key = (char *) bson_iter_key (&iter);
			value = (bson_value_t *) bson_iter_value (&iter);

			if (!strcmp (key, "_id")) {
				bson_oid_copy (&value->value.v_oid, &category->oid);
				bson_oid_to_string (&category->oid, category->id);
			}

			else if (!strcmp (key, "user")) {
				bson_oid_copy (&value->value.v_oid, &category->user_oid);
			}

			else if (!strcmp (key, "title") && value->value.v_utf8.str) {
				(void) strncpy (
					category->title,
					value->value.v_utf8.str,
					CATEGORY_TITLE_SIZE - 1
				);
			}

			else if (!strcmp (key, "description")) {
				(void) strncpy (
					category->description,
					value->value.v_utf8.str,
					CATEGORY_DESCRIPTION_SIZE - 1
				);
			}

			else if (!strcmp (key, "color")) {
				(void) strncpy (
					category->color,
					value->value.v_utf8.str,
					CATEGORY_COLOR_SIZE - 1
				);
			}

			else if (!strcmp (key, "date")) {
				category->date = (time_t) bson_iter_date_time (&iter) / 1000;
			}
		}
	}

}

static bson_t *category_query_oid (const bson_oid_t *oid) {

	bson_t *query = bson_new ();
	if (query) {
		(void) bson_append_oid (query, "_id", -1, oid);
	}

	return query;

}

static bson_t *category_query_by_oid_and_user (
	const bson_oid_t *oid, const bson_oid_t *user_oid
) {

	bson_t *category_query = bson_new ();
	if (category_query) {
		(void) bson_append_oid (category_query, "_id", -1, oid);
		(void) bson_append_oid (category_query, "user", -1, user_oid);
	}

	return category_query;

}

static bson_t *category_query_by_user (
	const bson_oid_t *user_oid
) {

	bson_t *query = bson_new ();
	if (query) {
		(void) bson_append_oid (query, "user", -1, user_oid);
	}

	return query;

}

bool category_exists (const bson_oid_t *oid) {

	return mongo_check (categories_model, category_query_oid (oid));

}

bool category_exists_with_user (
	const bson_oid_t *oid, const bson_oid_t *user_oid
) {

	return mongo_check (
		categories_model,
		category_query_by_oid_and_user (oid, user_oid)
	);

}

unsigned int category_get_by_oid (
	Category *category, const bson_oid_t *oid, const bson_t *query_opts
) {

	return mongo_find_one_with_opts (
		categories_model,
		category_query_oid (oid), query_opts,
		category
	);

}

unsigned int category_get_by_oid_and_user (
	Category *category,
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts
) {

	return mongo_find_one_with_opts (
		categories_model,
		category_query_by_oid_and_user (oid, user_oid), query_opts,
		category
	);

}

unsigned int category_get_by_oid_and_user_to_json (
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts,
	char **json, size_t *json_len
) {

	return mongo_find_one_with_opts_to_json (
		categories_model,
		category_query_by_oid_and_user (oid, user_oid), query_opts,
		json, json_len
	);

}

static bson_t *category_to_bson (const Category *category) {

	bson_t *doc = bson_new ();

	if (doc) {
		(void) bson_append_oid (doc, "_id", -1, &category->oid);

		(void) bson_append_oid (doc, "user", -1, &category->user_oid);

		(void) bson_append_utf8 (doc, "title", -1, category->title, -1);
		(void) bson_append_utf8 (doc, "description", -1, category->description, -1);
		(void) bson_append_utf8 (doc, "color", -1, category->color, -1);

		(void) bson_append_date_time (doc, "date", -1, category->date * 1000);
	}

	return doc;

}

static bson_t *category_update_bson (const Category *category) {

	bson_t *doc = bson_new ();

	if (doc) {
		bson_t set_doc = BSON_INITIALIZER;
		(void) bson_append_document_begin (doc, "$set", -1, &set_doc);
		(void) bson_append_utf8 (&set_doc, "title", -1, category->title, -1);
		(void) bson_append_utf8 (&set_doc, "description", -1, category->description, -1);
		(void) bson_append_utf8 (&set_doc, "color", -1, category->color, -1);
		(void) bson_append_document_end (doc, &set_doc);
	}

	return doc;

}

// get all the categories that are related to a user
mongoc_cursor_t *categories_get_all_by_user (
	const bson_oid_t *user_oid, const bson_t *opts
) {

	return mongo_find_all_cursor_with_opts (
		categories_model,
		category_query_by_user (user_oid), opts
	);

}

unsigned int categories_get_all_by_user_to_json (
	const bson_oid_t *user_oid, const bson_t *opts,
	char **json, size_t *json_len
) {

	return mongo_find_all_to_json (
		categories_model,
		category_query_by_user (user_oid), opts,
		"categories",
		json, json_len
	);

}

unsigned int category_insert_one (const Category *category) {

	return mongo_insert_one (
		categories_model, category_to_bson (category)
	);

}

unsigned int category_update_one (const Category *category) {

	return mongo_update_one (
		categories_model,
		category_query_oid (&category->oid),
		category_update_bson (category)
	);

}

unsigned int category_delete_one_by_oid_and_user (
	const bson_oid_t *oid, const bson_oid_t *user_oid
) {

	return mongo_delete_one (
		categories_model, category_query_by_oid_and_user (
			oid, user_oid
		)
	);

}
