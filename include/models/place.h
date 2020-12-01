#ifndef _MODELS_PLACE_H_
#define _MODELS_PLACE_H_

#include <time.h>

#include <mongoc/mongoc.h>
#include <bson/bson.h>

#include <cerver/types/types.h>
#include <cerver/types/string.h>

#include <cerver/cerver.h>

#define PLACE_ID_LEN				32
#define PLACE_NAME_LEN			    512
#define PLACE_DESCRIPTION_LEN	    1024

extern mongoc_collection_t *places_collection;

// opens handle to place collection
extern unsigned int places_collection_get (void);

extern void places_collection_close (void);

#define PLACE_TYPE_MAP(XX)					\
	XX(0,	NONE, 		None)				\
	XX(1,	LOCATION, 	Location)			\
	XX(2,	SITE, 		Site)

typedef enum PlaceType {

	#define XX(num, name, string) PLACE_TYPE_##name = num,
	PLACE_TYPE_MAP (XX)
	#undef XX

} PlaceType;

extern const char *place_type_to_string (PlaceType type);

typedef struct Place {

	bson_oid_t oid;
	char id[PLACE_ID_LEN];

	bson_oid_t user_oid;

	char name[PLACE_NAME_LEN];
	char description[PLACE_DESCRIPTION_LEN];

	PlaceType type;

	time_t date;

} Place;

extern void *place_new (void);

extern void place_delete (void *place_ptr);

extern void place_print (Place *place);

extern bson_t *place_query_oid (const bson_oid_t *oid);

extern const bson_t *place_find_by_oid (
	const bson_oid_t *oid, const bson_t *query_opts
);

extern u8 place_get_by_oid (
	Place *place, const bson_oid_t *oid, const bson_t *query_opts
);

extern const bson_t *place_find_by_oid_and_user (
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts
);

extern u8 place_get_by_oid_and_user (
	Place *place,
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts
);

#endif