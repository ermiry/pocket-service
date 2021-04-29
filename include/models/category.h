#ifndef _MODELS_CATEGORY_H_
#define _MODELS_CATEGORY_H_

#include <time.h>

#include <bson/bson.h>
#include <mongoc/mongoc.h>

#include <cerver/types/types.h>

#define CATEGORIES_COLL_NAME        "categories"

#define CATEGORY_ID_SIZE			32
#define CATEGORY_TITLE_SIZE			1024
#define CATEGORY_DESCRIPTION_SIZE	2048
#define CATEGORY_COLOR_SIZE			128

extern unsigned int categories_model_init (void);

extern void categories_model_end (void);

typedef struct Category {

	// category's unique id
	bson_oid_t oid;
	char id[CATEGORY_ID_SIZE];

	// reference to the owner of this category
	bson_oid_t user_oid;

	// how the user defined this transaction
	char title[CATEGORY_TITLE_SIZE];

	// a description added by the user to give extra information
	char description[CATEGORY_DESCRIPTION_SIZE];

	// the user can select a color
	// that will be used to display all matching transactions
	// in the mobile app
	char color[CATEGORY_COLOR_SIZE];

	// the date when the category was created
	time_t date;

} Category;

extern void *category_new (void);

extern void category_delete (void *category_ptr);

extern void category_print (Category *category);

extern bson_t *category_query_oid (const bson_oid_t *oid);

extern bson_t *category_query_by_oid_and_user (
	const bson_oid_t *oid, const bson_oid_t *user_oid
);

extern u8 category_get_by_oid (
	Category *category, const bson_oid_t *oid, const bson_t *query_opts
);

extern u8 category_get_by_oid_and_user (
	Category *category,
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts
);

extern u8 category_get_by_oid_and_user_to_json (
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts,
	char **json, size_t *json_len
);

// get all the categories that are related to a user
extern mongoc_cursor_t *categories_get_all_by_user (
	const bson_oid_t *user_oid, const bson_t *opts
);

extern unsigned int categories_get_all_by_user_to_json (
	const bson_oid_t *user_oid, const bson_t *opts,
	char **json, size_t *json_len
);

extern unsigned int category_insert_one (const Category *category);

extern unsigned int category_update_one (const Category *category);

extern unsigned int category_delete_one_by_oid_and_user (
	const bson_oid_t *oid, const bson_oid_t *user_oid
);

#endif