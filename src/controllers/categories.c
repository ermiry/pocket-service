#include <stdlib.h>

#include <time.h>

#include <cerver/types/string.h>

#include <cerver/collections/pool.h>

#include <cerver/http/json/json.h>

#include <cerver/utils/log.h>

#include "mongo.h"

#include "controllers/categories.h"

#include "models/category.h"

Pool *categories_pool = NULL;

const bson_t *category_no_user_query_opts = NULL;
DoubleList *category_no_user_select = NULL;

void pocket_category_delete (void *category_ptr);

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

	category_no_user_select = dlist_init (str_delete, str_comparator);
	(void) dlist_insert_after (category_no_user_select, dlist_end (category_no_user_select), str_new ("title"));
	(void) dlist_insert_after (category_no_user_select, dlist_end (category_no_user_select), str_new ("amount"));
	(void) dlist_insert_after (category_no_user_select, dlist_end (category_no_user_select), str_new ("date"));

	category_no_user_query_opts = mongo_find_generate_opts (category_no_user_select);

	if (category_no_user_query_opts) retval = 0;

	return retval;

}

unsigned int pocket_categories_init (void) {

	unsigned int errors = 0;

	errors |= pocket_categories_init_pool ();

	errors |= pocket_categories_init_query_opts ();

	return errors;

}

void pocket_categories_end (void) {

	bson_destroy ((bson_t *) category_no_user_query_opts);

	pool_delete (categories_pool);
	categories_pool = NULL;

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
				pocket_category_delete (category);
				category = NULL;
			}
		}
	}

	return category;

}

Category *pocket_category_create (
	const char *user_id,
	const char *title, const char *description,
	const char *color
) {

	Category *category = (Category *) pool_pop (categories_pool);
	if (category) {
		bson_oid_init (&category->oid, NULL);

		bson_oid_init_from_string (&category->user_oid, user_id);

		if (title) (void) strncpy (category->title, title, CATEGORY_TITLE_LEN);
		if (description) (void) strncpy (category->description, description, CATEGORY_DESCRIPTION_LEN);

		if (color) (void) strncpy (category->color, color, CATEGORY_COLOR_LEN);
		
		category->date = time (NULL);
	}

	return category;

}

void pocket_category_delete (void *category_ptr) {

	(void) memset (category_ptr, 0, sizeof (Category));
	(void) pool_push (categories_pool, category_ptr);

}