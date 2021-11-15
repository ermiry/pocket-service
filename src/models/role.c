#include <stdlib.h>
#include <stdio.h>

#include <bson/bson.h>
#include <mongoc/mongoc.h>

#include <cerver/collections/dlist.h>

#include <cmongo/collections.h>
#include <cmongo/crud.h>
#include <cmongo/model.h>
#include <cmongo/select.h>
#include <cmongo/types.h>

#include "models/role.h"

static CMongoModel *roles_model = NULL;

static void role_doc_parse (
	void *role_ptr, const bson_t *role_doc
);

unsigned int roles_model_init (void) {

	unsigned int retval = 1;

	roles_model = cmongo_model_create (ROLES_COLL_NAME);
	if (roles_model) {
		cmongo_model_set_parser (roles_model, role_doc_parse);

		retval = 0;
	}

	return retval;

}

void roles_model_end (void) {

	cmongo_model_delete (roles_model);

}

Role *role_new (void) {

	Role *role = (Role *) malloc (sizeof (Role));
	if (role) {
		(void) memset (role, 0, sizeof (Role));
	}

	return role;

}

void role_delete (void *role_ptr) {

	if (role_ptr) free (role_ptr);

}

void role_set_name (Role *role, const char *name) {

	(void) strncpy (role->name, name, ROLE_NAME_SIZE - 1);

}

void role_add_action (Role *role, const char *action_name) {

	if (role) {
		(void) strncpy (
			role->actions[role->n_actions],
			action_name,
			ROLE_ACTION_SIZE - 1
		);

		role->n_actions += 1;
	}

}

void role_remove_action (Role *role, const char *action_name) {

	if (role) {
		if (role->n_actions > 1) {
			unsigned int idx = 0;
			for (; idx < role->n_actions; idx++) {
				if (!strcmp (role->actions[idx], action_name)) {
					break;
				}
			}

			unsigned int stop = role->n_actions - 1;
			for (unsigned int i = idx; i < stop; i++) {
				(void) strncpy (
					role->actions[i],
					role->actions[i + 1],
					ROLE_ACTION_SIZE - 1
				);
			}
		}

		role->n_actions -= 1;
	}

}

Role *role_create (const char *name) {

	Role *role = role_new ();
	if (role) {
		bson_oid_init (&role->oid, NULL);
		bson_oid_to_string (&role->oid, role->id);

		role_set_name (role, name);
	}

	return role;

}

int role_comparator_by_name (
	const void *a, const void *b
) {

	return strcmp (((Role *) a)->name, ((Role *) b)->name);

}

bool role_check_action (const Role *role, const char *action) {

	bool retval = false;

	for (unsigned int i = 0; i < role->n_actions; i++) {
		if (!strcmp (role->actions[i], action)) {
			retval = true;
			break;
		}
	}

	return retval;

}

void role_print (const Role *role) {

	if (role) {
		(void) printf ("Name: %s\n", role->name);
		if (role->n_actions) {
			(void) printf ("Actions: \n");
			for (unsigned int i = 0; i < role->n_actions; i++) {
				(void) printf ("\t%s\n", role->actions[i]);
			}
		}

		else {
			(void) printf ("No actions!\n");
		}
	}

}

static void role_doc_parse_actions (
	Role *role, bson_iter_t *iter
) {

	const uint8_t *data = NULL;
	uint32_t len = 0;

	bson_iter_array (iter, &len, &data);

	bson_t *actions_array = bson_new_from_data (data, len);
	if (actions_array) {
		bson_iter_t array_iter = { 0 };
		if (bson_iter_init (&array_iter, actions_array)) {
			while (bson_iter_next (&array_iter)) {
				// const char *key = bson_iter_key (&array_iter);
				const bson_value_t *value = bson_iter_value (&array_iter);

				(void) strncpy (
					role->actions[role->n_actions],
					value->value.v_utf8.str,
					ROLE_ACTION_SIZE - 1
				);

				role->n_actions += 1;
			}
		}

		bson_destroy (actions_array);
	}

}

static void role_doc_parse (
	void *role_ptr, const bson_t *role_doc
) {

	Role *role = (Role *) role_ptr;

	bson_iter_t iter = { 0 };
	if (bson_iter_init (&iter, role_doc)) {
		while (bson_iter_next (&iter)) {
			const char *key = bson_iter_key (&iter);
			const bson_value_t *value = bson_iter_value (&iter);

			if (!strcmp (key, "_id")) {
				bson_oid_copy (&value->value.v_oid, &role->oid);
				bson_oid_to_string (&role->oid, role->id);
			}

			else if (!strcmp (key, "name") && value->value.v_utf8.str)
				(void) strncpy (
					role->name, value->value.v_utf8.str, ROLE_NAME_SIZE - 1
				);

			else if (!strcmp (key, "actions")) {
				role_doc_parse_actions (role, &iter);
			}
		}
	}

}

static bson_t *role_query_by_oid (
	const bson_oid_t *oid
) {

	bson_t *role_query = bson_new ();
	if (role_query) {
		(void) bson_append_oid (role_query, "_id", -1, oid);
	}

	return role_query;

}

static bson_t *role_query_by_name (
	const char *name
) {

	bson_t *role_query = bson_new ();
	if (role_query) {
		(void) bson_append_utf8 (role_query, "name", -1, name, -1);
	}

	return role_query;

}

DoubleList *roles_get_all (void) {

	DoubleList *roles = NULL;

	uint64_t n_docs = 0;
	mongoc_cursor_t *roles_cursor = mongo_find_all_cursor (
		roles_model, bson_new (), NULL, &n_docs
	);

	if (roles_cursor) {
		roles = dlist_init (role_delete, role_comparator_by_name);

		Role *role = NULL;
		const bson_t *role_doc = NULL;
		while (mongoc_cursor_next (roles_cursor, &role_doc)) {
			role = role_new ();
			if (role) {
				role_doc_parse (role, role_doc);

				(void) dlist_insert_after (
					roles,
					dlist_end (roles),
					role
				);
			}
		}

		mongoc_cursor_destroy (roles_cursor);
	}

	return roles;

}

unsigned int role_get_by_name (
	Role *role,
	const char *name
) {

	unsigned int retval = 1;

	if (role && name) {
		bson_t *role_query = role_query_by_name (name);
		if (role_query) {
			retval = mongo_find_one_with_opts (
				roles_model,
				role_query, NULL,
				role
			);
		}
	}

	return retval;

}
