#ifndef _MODELS_PLACE_H_
#define _MODELS_PLACE_H_

#include <time.h>

#include <bson/bson.h>
#include <mongoc/mongoc.h>

#include <cerver/types/types.h>
#include <cerver/types/string.h>

#include <cerver/cerver.h>

#define PLACES_COLL_NAME         	"places"

#define PLACE_ID_SIZE				32
#define PLACE_NAME_SIZE			    512
#define PLACE_DESCRIPTION_SIZE	    1024

#define PLACE_COLOR_SIZE			128

#define LOCATION_ADDRESS_SIZE		256
#define LOCATION_LAT_SIZE			32
#define LOCATION_LON_SIZE			32

#define SITE_LINK_SIZE				256
#define SITE_LOGO_SIZE				256

extern unsigned int places_model_init (void);

extern void places_model_end (void);

#define PLACE_TYPE_INVALID					3

#define PLACE_TYPE_MAP(XX)					\
	XX(0,	NONE, 		None)				\
	XX(1,	LOCATION, 	Location)			\
	XX(2,	SITE, 		Site)

typedef enum PlaceType {

	#define XX(num, name, string) PLACE_TYPE_##name = num,
	PLACE_TYPE_MAP (XX)
	#undef XX

} PlaceType;

extern const char *place_type_to_string (
	const PlaceType type
);

extern PlaceType place_type_from_string (
	const char *type_string
);

extern PlaceType place_type_from_value_string (
	const char *type_value_string
);

typedef struct Location {

	char address[LOCATION_ADDRESS_SIZE];
	char lat[LOCATION_LAT_SIZE];
	char lon[LOCATION_LON_SIZE];

} Location;

typedef struct Site {

	char link[SITE_LINK_SIZE];
	char logo[SITE_LOGO_SIZE];

} Site;

typedef struct Place {

	// place's unique id
	bson_oid_t oid;
	char id[PLACE_ID_SIZE];

	// reference to the user that registered this place
	bson_oid_t user_oid;

	// the name of the place
	char name[PLACE_NAME_SIZE];
	// a text providing additional information about the place
	char description[PLACE_DESCRIPTION_SIZE];

	// the place's type
	// location -> reference to a physicial place
	// link -> reference to a website or online store
	PlaceType type;

	Location location;
	Site site;

	char color[PLACE_COLOR_SIZE];

	// the date when the place was created
	time_t date;

} Place;

extern void *place_new (void);

extern void place_delete (void *place_ptr);

extern void place_print (const Place *place);

extern bson_t *place_query_oid (const bson_oid_t *oid);

extern bson_t *place_query_by_oid_and_user (
	const bson_oid_t *oid, const bson_oid_t *user_oid
);

extern u8 place_get_by_oid (
	Place *place, const bson_oid_t *oid, const bson_t *query_opts
);

extern u8 place_get_by_oid_and_user (
	Place *place,
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts
);

extern u8 place_get_by_oid_and_user_to_json (
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts,
	char **json, size_t *json_len
);

extern bson_t *place_update_bson (const Place *place);

// get all the places that are related to a user
extern mongoc_cursor_t *places_get_all_by_user (
	const bson_oid_t *user_oid, const bson_t *opts
);

extern char *places_get_all_by_user_to_json (
	const bson_oid_t *user_oid, const bson_t *opts,
	size_t *json_len
);

extern unsigned int place_insert_one (const Place *place);

extern unsigned int place_update_one (const Place *place);

extern unsigned int place_delete_one_by_oid_and_user (
	const bson_oid_t *oid, const bson_oid_t *user_oid
);

#endif