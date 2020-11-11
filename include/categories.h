#ifndef _POCKET_CATEGORIES_H_
#define _POCKET_CATEGORIES_H_

#include <bson/bson.h>

#include <cerver/collections/dlist.h>
#include <cerver/collections/pool.h>

#include "models/category.h"

#define DEFAULT_CATEGORIES_POOL_INIT			32

extern Pool *categories_pool;

extern const bson_t *category_no_user_query_opts;
extern DoubleList *category_no_user_select;

extern unsigned int pocket_categories_init (void);

extern void pocket_categories_end (void);

extern Category *pocket_category_create (
	const char *user_id,
	const char *title, const char *description,
	const char *color
);

extern void pocket_category_delete (void *category_ptr);

#endif