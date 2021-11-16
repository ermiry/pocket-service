#ifndef _MODELS_ACTION_H_
#define _MODELS_ACTION_H_

#include <bson/bson.h>
#include <mongoc/mongoc.h>

#include <cerver/collections/dlist.h>

#define ACTIONS_COLL_NAME  				"actions"

#define ACTION_ID_SIZE					32
#define ACTION_NAME_SIZE				128
#define ACTION_DESCRIPTION_SIZE			256

extern unsigned int actions_model_init (void);

extern void actions_model_end (void);

struct _RoleAction {

	bson_oid_t oid;
	char id[ACTION_ID_SIZE];

	char name[ACTION_NAME_SIZE];
	char description[ACTION_DESCRIPTION_SIZE];

};

typedef struct _RoleAction RoleAction;

extern RoleAction *action_new (void);

extern void action_delete (void *action_ptr);

extern RoleAction *action_create (
	const char *name, const char *description
);

extern int action_comparator_by_name (
	const void *a, const void *b
);

extern void action_print (const RoleAction *action);

extern DoubleList *actions_get_all (void);

extern unsigned int action_get_by_name (
	RoleAction *action,
	const char *name
);

extern unsigned int action_save (const RoleAction *action);

#endif