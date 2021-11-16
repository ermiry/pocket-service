#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <cerver/utils/log.h>

#include <cmongo/collections.h>
#include <cmongo/crud.h>
#include <cmongo/model.h>

#include "models/user.h"

static CMongoModel *users_model = NULL;

static void user_doc_parse (
	void *user_ptr, const bson_t *user_doc
);

unsigned int users_model_init (void) {

	unsigned int retval = 1;

	users_model = cmongo_model_create (USERS_COLL_NAME);
	if (users_model) {
		cmongo_model_set_parser (users_model, user_doc_parse);

		retval = 0;
	}

	return retval;

}

void users_model_end (void) {

	cmongo_model_delete (users_model);

}

void *user_new (void) {

	User *user = (User *) malloc (sizeof (User));
	if (user) {
		(void) memset (user, 0, sizeof (User));
	}

	return user;

}

void user_delete (void *user_ptr) {

	if (user_ptr) free (user_ptr);

}

void user_print (const User *user) {

	if (user) {
		(void) printf ("email: %s\n", user->email);
		(void) printf ("iat: %ld\n", user->iat);
		(void) printf ("id: %s\n", user->id);
		(void) printf ("name: %s\n", user->name);
		(void) printf ("role: %s\n", user->role);
		(void) printf ("username: %s\n", user->username);
	}

}

// parses a bson doc into a user model
static void user_doc_parse (
	void *user_ptr, const bson_t *user_doc
) {

	User *user = (User *) user_ptr;

	bson_iter_t iter = { 0 };
	if (bson_iter_init (&iter, user_doc)) {
		char *key = NULL;
		bson_value_t *value = NULL;
		while (bson_iter_next (&iter)) {
			key = (char *) bson_iter_key (&iter);
			value = (bson_value_t *) bson_iter_value (&iter);

			if (!strcmp (key, "_id")) {
				bson_oid_copy (&value->value.v_oid, &user->oid);
				bson_oid_to_string (&user->oid, user->id);
			}

			else if (!strcmp (key, "role")) {
				bson_oid_copy (&value->value.v_oid, &user->role_oid);
			}

			else if (!strcmp (key, "name") && value->value.v_utf8.str) {
				(void) strncpy (
					user->name,
					value->value.v_utf8.str,
					USER_NAME_SIZE - 1
				);
			}

			else if (!strcmp (key, "email") && value->value.v_utf8.str) {
				(void) strncpy (
					user->email,
					value->value.v_utf8.str,
					USER_EMAIL_SIZE - 1
				);
			}

			else if (!strcmp (key, "username") && value->value.v_utf8.str) {
				(void) strncpy (
					user->username,
					value->value.v_utf8.str,
					USER_USERNAME_SIZE - 1
				);
			}
		}
	}

}

static bson_t *user_query_by_oid (const bson_oid_t *oid) {

	bson_t *query = bson_new ();

	if (query) {
		(void) bson_append_oid (query, "_id", -1, oid);
	}

	return query;

}

static bson_t *user_query_by_email (const char *email) {

	bson_t *query = bson_new ();

	if (query) {
		(void) bson_append_utf8 (query, "email", -1, email, -1);
	}

	return query;

}

unsigned int user_get_by_oid (
	User *user, const bson_oid_t *oid, const bson_t *query_opts
) {

	return mongo_find_one_with_opts (
		users_model,
		user_query_by_oid (oid), query_opts,
		user
	);

}

bool user_check_by_email (const char *email) {

	return mongo_check (users_model, user_query_by_email (email));

}

// gets a user from the db by its email
unsigned int user_get_by_email (
	User *user, const char *email, const bson_t *query_opts
) {

	return mongo_find_one_with_opts (
		users_model,
		user_query_by_email (email), query_opts,
		user
	);

}
