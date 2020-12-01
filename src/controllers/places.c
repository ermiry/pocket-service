#include <stdlib.h>

#include <time.h>

#include <cerver/types/string.h>

#include <cerver/collections/pool.h>

#include <cerver/utils/log.h>

#include "controllers/places.h"

#include "models/place.h"

Pool *places_pool = NULL;

void pocket_place_delete (void *place_ptr);

static unsigned int pocket_places_init_pool (void) {

	unsigned int retval = 1;

	places_pool = pool_create (place_delete);
	if (places_pool) {
		pool_set_create (places_pool, place_new);
		pool_set_produce_if_empty (places_pool, true);
		if (!pool_init (places_pool, place_new, DEFAULT_PLACES_POOL_INIT)) {
			retval = 0;
		}

		else {
			cerver_log_error ("Failed to init places pool!");
		}
	}

	else {
		cerver_log_error ("Failed to create places pool!");
	}

	return retval;

}

unsigned int pocket_places_init (void) {

	unsigned int errors = 0;

	errors |= pocket_places_init_pool ();

	return errors;

}

void pocket_places_end (void) {

	pool_delete (places_pool);
	places_pool = NULL;

}

Place *pocket_place_get_by_id_and_user (
	const String *place_id, const bson_oid_t *user_oid
) {

	Place *place = NULL;

	if (place_id) {
		place = (Place *) pool_pop (places_pool);
		if (place) {
			bson_oid_init_from_string (&place->oid, place_id->str);

			if (place_get_by_oid_and_user (
				place,
				&place->oid, user_oid,
				NULL
			)) {
				pocket_place_delete (place);
				place = NULL;
			}
		}
	}

	return place;

}

Place *pocket_place_create (
	const char *user_id,
	const char *name, const char *description
) {

	Place *place = (Place *) pool_pop (places_pool);
	if (place) {
		bson_oid_init (&place->oid, NULL);

		bson_oid_init_from_string (&place->user_oid, user_id);

		if (name) (void) strncpy (place->name, name, PLACE_NAME_LEN);
		if (description) (void) strncpy (place->description, description, PLACE_DESCRIPTION_LEN);
		
		place->date = time (NULL);
	}

	return place;

}

void pocket_place_delete (void *place_ptr) {

	(void) memset (place_ptr, 0, sizeof (Place));
	(void) pool_push (places_pool, place_ptr);

}