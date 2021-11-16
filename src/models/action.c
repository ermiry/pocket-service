#include <stdlib.h>
#include <stdio.h>

#include <bson/bson.h>
#include <mongoc/mongoc.h>

#include <cerver/collections/dlist.h>

#include <cmongo/collections.h>
#include <cmongo/crud.h>
#include <cmongo/model.h>

#include "models/action.h"

static CMongoModel *actions_model = NULL;

static void action_doc_parse (
	void *action_ptr, const bson_t *action_doc
);

unsigned int actions_model_init (void) {

	unsigned int retval = 1;

	actions_model = cmongo_model_create (ACTIONS_COLL_NAME);
	if (actions_model) {
		cmongo_model_set_parser (actions_model, action_doc_parse);

		retval = 0;
	}

	return retval;

}

void actions_model_end (void) {

	cmongo_model_delete (actions_model);

}

RoleAction *action_new (void) {

	RoleAction *action = (RoleAction *) malloc (sizeof (RoleAction));
	if (action) {
		(void) memset (action, 0, sizeof (RoleAction));
	}

	return action;

}

void action_delete (void *action_ptr) {

	if (action_ptr) free (action_ptr);

}

RoleAction *action_create (
	const char *name, const char *description
) {

	RoleAction *action = action_new ();
	if (action) {
		bson_oid_init (&action->oid, NULL);
		bson_oid_to_string (&action->oid, action->id);

		(void) strncpy (action->name, name, ACTION_NAME_SIZE - 1);
		(void) strncpy (action->description, description, ACTION_DESCRIPTION_SIZE - 1);
	}

	return action;

}

int action_comparator_by_name (
	const void *a, const void *b
) {

	return strcmp (((RoleAction *) a)->name, ((RoleAction *) b)->name);

}

void action_print (const RoleAction *action) {

	if (action) {
		(void) printf ("Name: %s\n", action->name);
		(void) printf ("Description: %s\n", action->description);
	}

}

static void action_doc_parse (
	void *action_ptr, const bson_t *action_doc
) {

	RoleAction *action = (RoleAction *) action_ptr;

	bson_iter_t iter = { 0 };
	if (bson_iter_init (&iter, action_doc)) {
		while (bson_iter_next (&iter)) {
			const char *key = bson_iter_key (&iter);
			const bson_value_t *value = bson_iter_value (&iter);

			if (!strcmp (key, "_id")) {
				bson_oid_copy (&value->value.v_oid, &action->oid);
				bson_oid_to_string (&action->oid, action->id);
			}

			else if (!strcmp (key, "name") && value->value.v_utf8.str) {
				(void) strncpy (
					action->name, value->value.v_utf8.str, ACTION_NAME_SIZE - 1
				);
			}

			else if (!strcmp (key, "description") && value->value.v_utf8.str) {
				(void) strncpy (
					action->description, value->value.v_utf8.str, ACTION_DESCRIPTION_SIZE - 1
				);
			}
		}
	}

}

static bson_t *action_query_by_name (
	const char *name
) {

	bson_t *action_query = bson_new ();
	if (action_query) {
		(void) bson_append_utf8 (action_query, "name", -1, name, -1);
	}

	return action_query;

}

DoubleList *actions_get_all (void) {

	DoubleList *actions = NULL;

	uint64_t n_docs = 0;
	mongoc_cursor_t *actions_cursor = mongo_find_all_cursor (
		actions_model, bson_new (), NULL, &n_docs
	);

	if (actions_cursor) {
		actions = dlist_init (action_delete, action_comparator_by_name);

		RoleAction *action = NULL;

		const bson_t *action_doc = NULL;
		while (mongoc_cursor_next (actions_cursor, &action_doc)) {
			action = action_new ();
			if (action) {
				action_doc_parse (action, action_doc);

				(void) dlist_insert_after (
					actions,
					dlist_end (actions),
					action
				);
			}
		}

		mongoc_cursor_destroy (actions_cursor);
	}

	return actions;

}

unsigned int action_get_by_name (
	RoleAction *action,
	const char *name
) {

	unsigned int retval = 1;

	if (action && name) {
		bson_t *action_query = action_query_by_name (name);
		if (action_query) {
			retval = mongo_find_one_with_opts (
				actions_model,
				action_query, NULL,
				action
			);
		}
	}

	return retval;

}

static bson_t *action_bson_create (const RoleAction *action) {

	bson_t *doc = bson_new ();
	if (doc) {
		(void) bson_append_oid (doc, "_id", -1, &action->oid);

		(void) bson_append_utf8 (doc, "name", -1, action->name, -1);
		(void) bson_append_utf8 (doc, "description", -1, action->description, -1);
	}

	return doc;

}

unsigned int action_save (const RoleAction *action) {

	return mongo_insert_one (
		actions_model, action_bson_create (action)
	);

}
