#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <cerver/types/types.h>

#include <cerver/utils/log.h>

#include "mongo.h"

#include "models/place.h"

#define PLACES_COLL_NAME         				"places"

mongoc_collection_t *places_collection = NULL;

// opens handle to place collection
unsigned int places_collection_get (void) {

	unsigned int retval = 1;

	places_collection = mongo_collection_get (PLACES_COLL_NAME);
	if (places_collection) {
		cerver_log_debug ("Opened handle to places collection!");
		retval = 0;
	}

	else {
		cerver_log_error ("Failed to get handle to places collection!");
	}

	return retval;

}

void places_collection_close (void) {

	if (places_collection) mongoc_collection_destroy (places_collection);

}

const char *place_type_to_string (PlaceType type) {

	switch (type) {
		#define XX(num, name, string) case PLACE_TYPE_##name: return #string;
		PLACE_TYPE_MAP(XX)
		#undef XX
	}

	return place_type_to_string (PLACE_TYPE_NONE);

}

void *place_new (void) {

	Place *place = (Place *) malloc (sizeof (Place));
	if (place) {
		(void) memset (place, 0, sizeof (Place));
	}

	return place;

}

void place_delete (void *place_ptr) {

	if (place_ptr) free (place_ptr);

}

void place_print (Place *place) {

	if (place) {
		char buffer[128] = { 0 };
		bson_oid_to_string (&place->oid, buffer);
		(void) printf ("id: %s\n", buffer);

		(void) printf ("name: %s\n", place->name);
		(void) printf ("description: %s\n", place->description);
		(void) printf ("type: %s\n", place_type_to_string (place->type));

		(void) strftime (buffer, 128, "%d/%m/%y - %T", gmtime (&place->date));
		(void) printf ("date: %s GMT\n", buffer);
	}

}