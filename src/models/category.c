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