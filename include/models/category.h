#ifndef _MODELS_CATEGORY_H_
#define _MODELS_CATEGORY_H_

#include <time.h>

#include <mongoc/mongoc.h>
#include <bson/bson.h>

#include <cerver/types/types.h>

#define CATEGORY_TITLE_LEN			1024
#define CATEGORY_DESCRIPTION_LEN	2048
#define CATEGORY_COLOR_LEN			128

extern mongoc_collection_t *categories_collection;

// opens handle to category collection
extern unsigned int categories_collection_get (void);

extern void categories_collection_close (void);

typedef struct Category {

	// category's unique id
	bson_oid_t oid;

	// reference to the owner of this category
	bson_oid_t user_oid;

	// how the user defined this transaction
	char title[CATEGORY_TITLE_LEN];

	// a description added by the user to give extra information
	char description[CATEGORY_DESCRIPTION_LEN];

	// the user can select a color
	// that will be used to display all matching transactions
	// in the mobile app
	char color[CATEGORY_COLOR_LEN];

	// the date when the category was created
	time_t date;

} Category;

extern void *category_new (void);

extern void category_delete (void *category_ptr);

extern void category_print (Category *category);

extern bson_t *category_query_oid (const bson_oid_t *oid);

extern const bson_t *category_find_by_oid (
	const bson_oid_t *oid, const bson_t *query_opts
);

extern u8 category_get_by_oid (
	Category *category, const bson_oid_t *oid, const bson_t *query_opts
);

extern const bson_t *category_find_by_oid_and_user (
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts
);

extern u8 category_get_by_oid_and_user (
	Category *category,
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts
);

extern bson_t *category_to_bson (Category *category);

extern bson_t *category_update_bson (Category *category);

// get all the categories that are related to a user
extern mongoc_cursor_t *categories_get_all_by_user (
	const bson_oid_t *user_oid, const bson_t *opts
);

#endif