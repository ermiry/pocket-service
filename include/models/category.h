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

	bson_oid_t oid;

	bson_oid_t user_oid;

	char title[CATEGORY_TITLE_LEN];
	char description[CATEGORY_DESCRIPTION_LEN];
	char color[CATEGORY_COLOR_LEN];
	time_t date;

} Category;

extern void *category_new (void);

extern void category_delete (void *category_ptr);

extern void category_print (Category *category);

#endif