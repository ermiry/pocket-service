#ifndef _MODELS_PLACE_H_
#define _MODELS_PLACE_H_

#include <time.h>

#include <bson/bson.h>
#include <mongoc/mongoc.h>

#include <cerver/types/types.h>
#include <cerver/types/string.h>

#include <cerver/cerver.h>

#define PLACE_ID_LEN				32
#define PLACE_NAME_LEN			    512
#define PLACE_DESCRIPTION_LEN	    1024

#define LOCATION_ADDRESS_LEN		256
#define LOCATION_LAT_LEN			32
#define LOCATION_LON_LEN			32

#define SITE_LINK_LEN				256
#define SITE_LOGO_LEN				256

extern unsigned int places_model_init (void);

extern void places_model_end (void);

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

typedef struct Location {

	// location's unique id
	bson_oid_t oid;

	char address[LOCATION_ADDRESS_LEN];
	char lat[LOCATION_LAT_LEN];
	char lon[LOCATION_LON_LEN];

} Location;

typedef struct Link {

	// location's unique id
	bson_oid_t oid;

	char link[SITE_LINK_LEN];
	char logo[SITE_LOGO_LEN];

} Link;

typedef struct Place {

	// place's unique id
	bson_oid_t oid;
	char id[PLACE_ID_LEN];

	// reference to the user that registered this place
	bson_oid_t user_oid;

	// the name of the place
	char name[PLACE_NAME_LEN];
	// a text providing additional information about the place
	char description[PLACE_DESCRIPTION_LEN];

	// the place's type
	// location -> reference to a physicial place
	// link -> reference to a website or online store
	PlaceType type;

	Location location;
	Link link;

	// the date when the place was created
	time_t date;

} Place;

extern void *place_new (void);

extern void place_delete (void *place_ptr);

extern void place_print (Place *place);

extern bson_t *place_query_oid (const bson_oid_t *oid);

extern bson_t *place_query_by_oid_and_user (
	const bson_oid_t *oid, const bson_oid_t *user_oid
);

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

extern bson_t *place_to_bson (const Place *place);

extern bson_t *place_update_bson (const Place *place);

// get all the places that are related to a user
extern mongoc_cursor_t *places_get_all_by_user (
	const bson_oid_t *user_oid, const bson_t *opts
);

extern unsigned int place_insert_one (const Place *place);

extern unsigned int place_update_one (const Place *place);

extern unsigned int place_delete_one_by_oid_and_user (
	const bson_oid_t *oid, const bson_oid_t *user_oid
);

#endif