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

static void place_doc_parse (
	Place *place, const bson_t *place_doc
) {

	bson_iter_t iter = { 0 };
	if (bson_iter_init (&iter, place_doc)) {
		char *key = NULL;
		bson_value_t *value = NULL;
		while (bson_iter_next (&iter)) {
			key = (char *) bson_iter_key (&iter);
			value = (bson_value_t *) bson_iter_value (&iter);

			if (!strcmp (key, "_id")) {
				bson_oid_copy (&value->value.v_oid, &place->oid);
				bson_oid_to_string (&place->oid, place->id);
			}

			else if (!strcmp (key, "user"))
				bson_oid_copy (&value->value.v_oid, &place->user_oid);

			else if (!strcmp (key, "name") && value->value.v_utf8.str) 
				(void) strncpy (place->name, value->value.v_utf8.str, PLACE_NAME_LEN);

			else if (!strcmp (key, "description")) 
				(void) strncpy (place->description, value->value.v_utf8.str, PLACE_DESCRIPTION_LEN);

			else if (!strcmp (key, "type"))
				place->type = value->value.v_int32;

			else if (!strcmp (key, "date")) 
				place->date = (time_t) bson_iter_date_time (&iter) / 1000;
		}
	}

}

bson_t *place_query_oid (const bson_oid_t *oid) {

	bson_t *query = NULL;

	if (oid) {
		query = bson_new ();
		if (query) {
			(void) bson_append_oid (query, "_id", -1, oid);
		}
	}

	return query;

}

const bson_t *place_find_by_oid (
	const bson_oid_t *oid, const bson_t *query_opts
) {

	const bson_t *retval = NULL;

	bson_t *place_query = bson_new ();
	if (place_query) {
		(void) bson_append_oid (place_query, "_id", -1, oid);
		retval = mongo_find_one_with_opts (places_collection, place_query, query_opts);
	}

	return retval;

}

u8 place_get_by_oid (
	Place *place, const bson_oid_t *oid, const bson_t *query_opts
) {

	u8 retval = 1;

	if (place) {
		const bson_t *place_doc = place_find_by_oid (oid, query_opts);
		if (place_doc) {
			place_doc_parse (place, place_doc);
			bson_destroy ((bson_t *) place_doc);

			retval = 0;
		}
	}

	return retval;

}

const bson_t *place_find_by_oid_and_user (
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts
) {

	const bson_t *retval = NULL;

	bson_t *place_query = bson_new ();
	if (place_query) {
		(void) bson_append_oid (place_query, "_id", -1, oid);
		(void) bson_append_oid (place_query, "user", -1, user_oid);

		retval = mongo_find_one_with_opts (places_collection, place_query, query_opts);
	}

	return retval;

}

u8 place_get_by_oid_and_user (
	Place *place,
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts
) {

	u8 retval = 1;

	if (place) {
		const bson_t *place_doc = place_find_by_oid_and_user (oid, user_oid, query_opts);
		if (place_doc) {
			place_doc_parse (place, place_doc);
			bson_destroy ((bson_t *) place_doc);

			retval = 0;
		}
	}

	return retval;

}

bson_t *place_to_bson (Place *place) {

    bson_t *doc = NULL;

    if (place) {
        doc = bson_new ();
        if (doc) {
            (void) bson_append_oid (doc, "_id", -1, &place->oid);

			(void) bson_append_oid (doc, "user", -1, &place->user_oid);

			(void) bson_append_utf8 (doc, "name", -1, place->name, -1);
			(void) bson_append_utf8 (doc, "description", -1, place->description, -1);

			(void) bson_append_int32 (doc, "type", -1, place->type);

			(void) bson_append_date_time (doc, "date", -1, place->date * 1000);
        }
    }

    return doc;

}

bson_t *place_update_bson (Place *place) {

	bson_t *doc = NULL;

    if (place) {
        doc = bson_new ();
        if (doc) {
			bson_t set_doc = { 0 };
			(void) bson_append_document_begin (doc, "$set", -1, &set_doc);

			(void) bson_append_utf8 (&set_doc, "name", -1, place->name, -1);
			(void) bson_append_utf8 (&set_doc, "description", -1, place->description, -1);
			
			(void) bson_append_document_end (doc, &set_doc);
        }
    }

    return doc;

}

// get all the places that are related to a user
mongoc_cursor_t *places_get_all_by_user (
	const bson_oid_t *user_oid, const bson_t *opts
) {

	mongoc_cursor_t *retval = NULL;

	if (user_oid && opts) {
		bson_t *query = bson_new ();
		if (query) {
			(void) bson_append_oid (query, "user", -1, user_oid);

			retval = mongo_find_all_cursor_with_opts (
				places_collection,
				query, opts
			);
		}
	}

	return retval;

}