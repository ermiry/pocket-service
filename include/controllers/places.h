#ifndef _POCKET_PLACES_H_
#define _POCKET_PLACES_H_

#include <bson/bson.h>

#include <cerver/types/string.h>

#include <cerver/collections/dlist.h>
#include <cerver/collections/pool.h>

#include "models/place.h"

#define DEFAULT_PLACES_POOL_INIT			32

extern Pool *places_pool;

extern const bson_t *place_no_user_query_opts;
extern DoubleList *place_no_user_select;

extern unsigned int pocket_places_init (void);

extern void pocket_places_end (void);

extern Place *pocket_place_get_by_id_and_user (
	const String *place_id, const bson_oid_t *user_oid
);

extern Place *pocket_place_create (
	const char *user_id,
	const char *name, const char *description
);

extern void pocket_place_delete (void *place_ptr);

#endif