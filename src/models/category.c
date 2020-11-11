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