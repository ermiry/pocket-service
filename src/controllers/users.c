#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cerver/types/string.h>

#include <cerver/collections/pool.h>

#include <cerver/handler.h>

#include <cerver/http/json/json.h>

#include <cerver/utils/log.h>

#include "pocket.h"

#include "controllers/roles.h"
#include "controllers/users.h"

#include "models/user.h"

static Pool *users_pool = NULL;

static unsigned int pocket_users_init_pool (void) {

	unsigned int retval = 1;

	users_pool = pool_create (user_delete);
	if (users_pool) {
		pool_set_create (users_pool, user_new);
		pool_set_produce_if_empty (users_pool, true);
		if (!pool_init (users_pool, user_new, DEFAULT_USERS_POOL_INIT)) {
			retval = 0;
		}

		else {
			cerver_log_error ("Failed to init users pool!");
		}
	}

	else {
		cerver_log_error ("Failed to create users pool!");
	}

	return retval;

}

unsigned int pocket_users_init (void) {

	unsigned int errors = 0;

	errors |= pocket_users_init_pool ();

	return errors;

}

void pocket_users_end (void) {

	pool_delete (users_pool);
	users_pool = NULL;

}

User *pocket_user_get (void) {

	return (User *) pool_pop (users_pool);

}

User *pocket_user_get_by_email (const char *email) {

	User *user = NULL;
	if (email) {
		user = (User *) pool_pop (users_pool);
		if (user) {
			if (user_get_by_email (user, email, NULL)) {
				(void) pool_push (users_pool, user);
				user = NULL;
			}
		}
	}

	return user;

}

bool pocket_user_check_by_email (const char *email) {

	return user_check_by_email (email);

}

// {
//   "email": "erick.salas@ermiry.com",
//   "iat": 1596532954
//   "id": "5eb2b13f0051f70011e9d3af",
//   "name": "Erick Salas",
//   "role": "god",
//   "username": "erick"
// }
void *pocket_user_parse_from_json (void *user_json_ptr) {

	json_t *user_json = (json_t *) user_json_ptr;

	User *user = user_new ();
	if (user) {
		const char *email = NULL;
		const char *id = NULL;
		const char *name = NULL;
		const char *role = NULL;
		const char *username = NULL;

		if (!json_unpack (
			user_json,
			"{s:s, s:i, s:s, s:s, s:s, s:s}",
			"email", &email,
			"iat", &user->iat,
			"id", &id,
			"name", &name,
			"role", &role,
			"username", &username
		)) {
			(void) strncpy (user->email, email, USER_EMAIL_SIZE - 1);
			(void) strncpy (user->id, id, USER_ID_SIZE - 1);
			(void) strncpy (user->name, name, USER_NAME_SIZE - 1);
			(void) strncpy (user->role, role, USER_ROLE_SIZE - 1);
			(void) strncpy (user->username, username, USER_USERNAME_SIZE - 1);

			bson_oid_init_from_string (&user->oid, user->id);

			// if (RUNTIME == RUNTIME_TYPE_DEVELOPMENT) {
			// 	user_print (user);
			// }
		}

		else {
			cerver_log_error ("user_parse_from_json () - json_unpack () has failed!");

			(void) pool_push (users_pool, user);
			user = NULL;
		}
	}

	return user;

}

void pocket_user_delete (void *user_ptr) {

	if (user_ptr)	 {
		(void) memset (user_ptr, 0, sizeof (User));
		(void) pool_push (users_pool, user_ptr);
	}

}
