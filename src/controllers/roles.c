#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <cmongo/crud.h>
#include <cmongo/select.h>

#include <cerver/collections/dlist.h>

#include <cerver/utils/log.h>

#include "models/role.h"

static DoubleList *roles = NULL;

const Role *pocket_role_get_by_name (
	const char *role_name
);

unsigned int pocket_roles_init (void) {

	unsigned int retval = 1;

	roles = roles_get_all ();
	if (roles) {
		cerver_log_success ("Got roles from db!");

		retval = 0;
	}

	else {
		cerver_log_error ("Failed to get roles from db!");
	}

	return retval;

}

void pocket_roles_end (void) {

	dlist_delete (roles);

}

const Role *pocket_roles_get_by_oid (
	const bson_oid_t *role_oid
) {

	const Role *retval = NULL;

	if (role_oid) {
		for (ListElement *le = dlist_start (roles); le; le = le->next) {
			if (!bson_oid_compare (&((Role *) le->data)->oid, role_oid)) {
				retval = ((Role *) le->data);
				break;
			}
		}
	}

	return retval;

}

const Role *pocket_roles_get_by_name (
	const char *role_name
) {

	const Role *retval = NULL;

	if (role_name) {
		for (ListElement *le = dlist_start (roles); le; le = le->next) {
			if (!strcmp (((Role *) le->data)->name, role_name)) {
				retval = ((Role *) le->data);
				break;
			}
		}
	}

	return retval;

}

const char *pocket_roles_name_get_by_oid (
	const bson_oid_t *role_oid
) {

	const char *retval = NULL;

	const Role *role = pocket_roles_get_by_oid (role_oid);
	if (role) {
		retval = role->name;
	}

	return retval;

}

bool pocket_role_search_and_check_action (
	const char *role_name, const char *action_name
) {

	bool retval = false;

	if (role_name && action_name) {
		Role *role = NULL;
		for (ListElement *le = dlist_start (roles); le; le = le->next) {
			role = (Role *) le->data;
			if (!strcmp (role->name, role_name)) {
				// search action in role
				for (unsigned int idx = 0; idx < role->n_actions; idx++) {
					if (!strcmp (role->actions[idx], action_name)) {
						retval = true;
						break;
					}
				}

				break;
			}
		}
	}

	return retval;

}
