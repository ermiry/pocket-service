#include <stdlib.h>
#include <string.h>

#include <cerver/types/types.h>
#include <cerver/types/string.h>

#include <cerver/collections/dlist.h>

#include <cerver/http/json/json.h>

#include <cerver/utils/log.h>

#include "mongo.h"

#include "models/user.h"

#define USERS_COLL_NAME         				"users"

mongoc_collection_t *users_collection = NULL;

// opens handle to user collection
unsigned int users_collection_get (void) {

	unsigned int retval = 1;

	users_collection = mongo_collection_get (USERS_COLL_NAME);
	if (users_collection) {
		retval = 0;
	}

	else {
		cerver_log_error ("Failed to get handle to users collection!");
	}

	return retval;

}

void users_collection_close (void) {

	if (users_collection) mongoc_collection_destroy (users_collection);

}

void *user_new (void) {

	User *user = (User *) malloc (sizeof (User));
	if (user) {
		memset (user, 0, sizeof (User));
	}

	return user;

}

void user_delete (void *user_ptr) {

	if (user_ptr) free (user_ptr);

}

void user_print (User *user) {

	if (user) {
		printf ("email: %s\n", user->email);
		printf ("iat: %ld\n", user->iat);
		printf ("id: %s\n", user->id);
		printf ("name: %s\n", user->name);
		printf ("role: %s\n", user->role);
		printf ("username: %s\n", user->username);
	}

}

// parses a bson doc into a user model
static void user_doc_parse (User *user, const bson_t *user_doc) {

	bson_iter_t iter = { 0 };
	if (bson_iter_init (&iter, user_doc)) {
		char *key = NULL;
		bson_value_t *value = NULL;
		while (bson_iter_next (&iter)) {
			key = (char *) bson_iter_key (&iter);
			value = (bson_value_t *) bson_iter_value (&iter);

			if (!strcmp (key, "_id")) {
				bson_oid_copy (&value->value.v_oid, &user->oid);
			}

			else if (!strcmp (key, "role")) {
				bson_oid_copy (&value->value.v_oid, &user->role_oid);
			}

			else if (!strcmp (key, "name") && value->value.v_utf8.str) 
				strncpy (user->name, value->value.v_utf8.str, USER_NAME_LEN);

			else if (!strcmp (key, "email") && value->value.v_utf8.str) 
				strncpy (user->email, value->value.v_utf8.str, USER_EMAIL_LEN);

			else if (!strcmp (key, "username") && value->value.v_utf8.str) 
				strncpy (user->username, value->value.v_utf8.str, USER_USERNAME_LEN);

			else if (!strcmp (key, "password") && value->value.v_utf8.str)
				strncpy (user->password, value->value.v_utf8.str, USER_PASSWORD_LEN);
		}
	}

}

// get a user doc from the db by email
static const bson_t *user_find_by_email (
	const String *email, const bson_t *query_opts
) {

	const bson_t *retval = NULL;

	bson_t *user_query = bson_new ();
	if (user_query) {
		bson_append_utf8 (user_query, "email", -1, email->str, email->len);
		retval = mongo_find_one_with_opts (users_collection, user_query, query_opts);
	}

	return retval;    

}

// gets a user from the db by its email
u8 user_get_by_email (
	User *user, const String *email, const bson_t *query_opts
) {

	u8 retval = 1;

	if (email) {
		const bson_t *user_doc = user_find_by_email (email, query_opts);
		if (user_doc) {
			user_doc_parse (user, user_doc);
			bson_destroy ((bson_t *) user_doc);
		}
	}

	return retval;

}

// get a user doc from the db by username
static const bson_t *user_find_by_username (
	const String *username, const bson_t *query_opts
) {

	const bson_t *retval = NULL;

	bson_t *user_query = bson_new ();
	if (user_query) {
		bson_append_utf8 (user_query, "username", -1, username->str, username->len);
		retval = mongo_find_one_with_opts (users_collection, user_query, query_opts);
	}

	return retval;    

}

// gets a user from the db by its username
u8 user_get_by_username (
	User *user, const String *username, const bson_t *query_opts
) {

	u8 retval = 1;

	if (username) {
		const bson_t *user_doc = user_find_by_username (username, query_opts);
		if (user_doc) {
			user_doc_parse (user, user_doc);
			bson_destroy ((bson_t *) user_doc);
		}
	}

	return retval;

}

bson_t *user_bson_create (User *user) {

	bson_t *doc = NULL;

	if (user) {
		doc = bson_new ();
		if (doc) {
			bson_append_oid (doc, "_id", -1, &user->oid);

			if (user->name) bson_append_utf8 (doc, "name", -1, user->name, -1);
			if (user->username) bson_append_utf8 (doc, "username", -1, user->username, -1);
			if (user->email) bson_append_utf8 (doc, "email", -1, user->email, -1);
			if (user->password) bson_append_utf8 (doc, "password", -1, user->password, -1);

			bson_append_oid (doc, "role", -1, &user->role_oid);
		}
	}

	return doc;

}