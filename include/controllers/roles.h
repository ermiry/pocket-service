#ifndef _POCKET_ROLES_H_
#define _POCKET_ROLES_H_

#include <bson/bson.h>

#include "models/role.h"

extern const Role *common_role;

extern unsigned int pocket_roles_init (void);

extern void pocket_roles_end (void);

extern const Role *pocket_role_get_by_oid (
	const bson_oid_t *role_oid
);

extern const Role *pocket_role_get_by_name (
	const char *role_name
);

extern const char *pocket_role_name_get_by_oid (
	const bson_oid_t *role_oid
);

#endif