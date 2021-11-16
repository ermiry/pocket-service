#ifndef _POCKET_PLACES_H_
#define _POCKET_PLACES_H_

#include <bson/bson.h>

#include <cerver/types/string.h>

#include <cerver/collections/pool.h>

#include "errors.h"

#include "models/place.h"
#include "models/user.h"

#define PLACES_POOL_INIT			32

struct _HttpResponse;

extern Pool *places_pool;

extern const bson_t *place_no_user_query_opts;

extern struct _HttpResponse *no_user_places;
extern struct _HttpResponse *no_user_place;

extern struct _HttpResponse *place_created_success;
extern struct _HttpResponse *place_created_bad;
extern struct _HttpResponse *place_deleted_success;
extern struct _HttpResponse *place_deleted_bad;

extern unsigned int pocket_places_init (void);

extern void pocket_places_end (void);

extern unsigned int pocket_places_get_all_by_user (
	const bson_oid_t *user_oid,
	char **json, size_t *json_len
);

extern Place *pocket_place_get_by_id_and_user (
	const String *place_id, const bson_oid_t *user_oid
);

extern u8 pocket_place_get_by_id_and_user_to_json (
	const char *place_id, const bson_oid_t *user_oid,
	const bson_t *query_opts,
	char **json, size_t *json_len
);

extern PocketError pocket_place_create (
	const User *user, const String *request_body
);

extern PocketError pocket_place_update (
	const User *user, const String *place_id,
	const String *request_body
);

extern PocketError pocket_place_delete (
	const User *user, const String *place_id
);

extern void pocket_place_return (void *place_ptr);

#endif