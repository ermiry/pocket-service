#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <cerver/types/types.h>

#include <cerver/utils/log.h>

#include "mongo.h"

#include "models/category.h"

#define CATEGORIES_COLL_NAME         				"categories"

mongoc_collection_t *categories_collection = NULL;

// opens handle to category collection
unsigned int categories_collection_get (void) {

	unsigned int retval = 1;

	categories_collection = mongo_collection_get (CATEGORIES_COLL_NAME);
	if (categories_collection) {
		retval = 0;
	}

	else {
		cerver_log_error ("Failed to get handle to categories collection!");
	}

	return retval;

}

void categories_collection_close (void) {

	if (categories_collection) mongoc_collection_destroy (categories_collection);

}

void *category_new (void) {

	Category *category = (Category *) malloc (sizeof (Category));
	if (category) {
		memset (category, 0, sizeof (Category));
	}

	return category;

}

void category_delete (void *category_ptr) {

	if (category_ptr) free (category_ptr);

}

void category_print (Category *category) {

	if (category) {
		char buffer[128] = { 0 };
		bson_oid_to_string (&category->oid, buffer);
		printf ("id: %s\n", buffer);

		printf ("title: %s\n", category->title);
		printf ("description: %s\n", category->description);
		printf ("color: %s\n", category->color);

		strftime (buffer, 128, "%d/%m/%y - %T", gmtime (&category->date));
		printf ("date: %s GMT\n", buffer);
	}

}

static void category_doc_parse (Category *category, const bson_t *category_doc) {

	bson_iter_t iter = { 0 };
	if (bson_iter_init (&iter, category_doc)) {
		char *key = NULL;
		bson_value_t *value = NULL;
		while (bson_iter_next (&iter)) {
			key = (char *) bson_iter_key (&iter);
			value = (bson_value_t *) bson_iter_value (&iter);

			if (!strcmp (key, "_id"))
				bson_oid_copy (&value->value.v_oid, &category->oid);

			else if (!strcmp (key, "user"))
				bson_oid_copy (&value->value.v_oid, &category->user_oid);

			else if (!strcmp (key, "title") && value->value.v_utf8.str) 
				(void) strncpy (category->title, value->value.v_utf8.str, CATEGORY_TITLE_LEN);

			else if (!strcmp (key, "description")) 
				(void) strncpy (category->description, value->value.v_utf8.str, CATEGORY_DESCRIPTION_LEN);

			else if (!strcmp (key, "color")) 
				(void) strncpy (category->color, value->value.v_utf8.str, CATEGORY_COLOR_LEN);

			else if (!strcmp (key, "date")) 
				category->date = (time_t) bson_iter_date_time (&iter) / 1000;
		}
	}

}

const bson_t *category_find_by_oid (
	const bson_oid_t *oid, const bson_t *query_opts
) {

	const bson_t *retval = NULL;

	bson_t *category_query = bson_new ();
	if (category_query) {
		bson_append_oid (category_query, "_id", -1, oid);
		retval = mongo_find_one_with_opts (categories_collection, category_query, query_opts);
	}

	return retval;

}

u8 category_get_by_oid (
	Category *category, const bson_oid_t *oid, const bson_t *query_opts
) {

	u8 retval = 1;

	if (category) {
		const bson_t *category_doc = category_find_by_oid (oid, query_opts);
		if (category_doc) {
			category_doc_parse (category, category_doc);
			bson_destroy ((bson_t *) category_doc);

			retval = 0;
		}
	}

	return retval;

}

const bson_t *category_find_by_oid_and_user (
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts
) {

	const bson_t *retval = NULL;

	bson_t *category_query = bson_new ();
	if (category_query) {
		bson_append_oid (category_query, "_id", -1, oid);
		bson_append_oid (category_query, "user", -1, user_oid);

		retval = mongo_find_one_with_opts (categories_collection, category_query, query_opts);
	}

	return retval;

}

u8 category_get_by_oid_and_user (
	Category *category,
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts
) {

	u8 retval = 1;

	if (category) {
		const bson_t *category_doc = category_find_by_oid_and_user (oid, user_oid, query_opts);
		if (category_doc) {
			category_doc_parse (category, category_doc);
			bson_destroy ((bson_t *) category_doc);

			retval = 0;
		}
	}

	return retval;

}

bson_t *category_to_bson (Category *category) {

    bson_t *doc = NULL;

    if (category) {
        doc = bson_new ();
        if (doc) {
            bson_append_oid (doc, "_id", -1, &category->oid);

			bson_append_oid (doc, "user", -1, &category->user_oid);

			bson_append_utf8 (doc, "title", -1, category->title, -1);
			bson_append_utf8 (doc, "description", -1, category->description, -1);
			bson_append_utf8 (doc, "color", -1, category->color, -1);

			bson_append_date_time (doc, "date", -1, category->date * 1000);
        }
    }

    return doc;

}


// get all the categories that are related to a user
mongoc_cursor_t *categories_get_all_by_user (
	const bson_oid_t *user_oid, const bson_t *opts
) {

	mongoc_cursor_t *retval = NULL;

	if (user_oid && opts) {
		bson_t *query = bson_new ();
		if (query) {
			bson_append_oid (query, "user", -1, user_oid);

			retval = mongo_find_all_cursor_with_opts (
				categories_collection,
				query, opts
			);
		}
	}

	return retval;

}