#ifndef _MODELS_ROLE_H_
#define _MODELS_ROLE_H_

#include <stdbool.h>

#include <bson/bson.h>
#include <mongoc/mongoc.h>

#include <cerver/collections/dlist.h>

#include <cmongo/select.h>

#define ROLES_COLL_NAME  		"roles"

#define ROLE_ID_SIZE			32
#define ROLE_NAME_SIZE			128
#define ROLE_ACTIONS_SIZE		32
#define ROLE_ACTION_SIZE		64

extern unsigned int roles_model_init (void);

extern void roles_model_end (void);

struct _Role {

	bson_oid_t oid;
	char id[ROLE_ID_SIZE];

	char name[ROLE_NAME_SIZE];

	unsigned int n_actions;
	char actions[ROLE_ACTIONS_SIZE][ROLE_ACTION_SIZE];

};

typedef struct _Role Role;

extern Role *role_new (void);

extern void role_delete (void *role_ptr);

extern void role_set_name (
	Role *role, const char *name
);

extern void role_add_action (
	Role *role, const char *action_name
);

extern void role_remove_action (
	Role *role, const char *action_name
);

extern Role *role_create (const char *name);

extern int role_comparator_by_name (
	const void *a, const void *b
);

extern bool role_check_action (
	const Role *role, const char *action
);

extern void role_print (const Role *role);

extern DoubleList *roles_get_all (void);

extern unsigned int role_get_by_name (
	Role *role,
	const char *name
);

#endif