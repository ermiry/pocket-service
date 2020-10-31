#include <stdlib.h>
#include <string.h>

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

User *user_new (void) {

	User *user = (User *) malloc (sizeof (User));
	if (user) {
		user->id = NULL;
		memset (&user->oid, 0, sizeof (bson_oid_t));

		user->name = NULL;
		user->username = NULL;
		user->email = NULL;
		user->password = NULL;

		user->role = NULL;
		memset (&user->role_oid, 0, sizeof (bson_oid_t));
		
		user->iat = 0;
	}

	return user;

}

void user_delete (void *user_ptr) {

	if (user_ptr) {
		User *user = (User *) user_ptr;

		str_delete (user->id);

		str_delete (user->name);
		str_delete (user->username);
		str_delete (user->email);
		str_delete (user->password);

		str_delete (user->role);

		free (user_ptr);
	}

}

User *user_create (
	const char *name,
	const char *username,
	const char *email,
	const char *password,
	const bson_oid_t *role_oid
) {

	User *user = user_new ();
	if (user) {
		user->name = str_new (name);
		user->username = str_new (username);
		user->email = str_new (email);
		user->password = str_new (password);
		bson_oid_copy (role_oid, &user->role_oid);
	}

	return user;

}

int user_comparator (const void *a, const void *b) {

	return strcmp (((User *) a)->username->str, ((User *) b)->username->str);

}

void user_print (User *user) {

	if (user) {
		printf ("id: %s\n", user->id->str);
		printf ("name: %s\n", user->name->str);
		printf ("username: %s\n", user->username->str);
		printf ("email: %s\n", user->email->str);
		printf ("role: %s\n", user->role->str);
	}

}

// parses a bson doc into a user model
static User *user_doc_parse (const bson_t *user_doc) {

	User *user = NULL;

	if (user_doc) {
		user = user_new ();
		if (user) {
			bson_iter_t iter = { 0 };
			if (bson_iter_init (&iter, user_doc)) {
				while (bson_iter_next (&iter)) {
					const char *key = bson_iter_key (&iter);
					const bson_value_t *value = bson_iter_value (&iter);

					if (!strcmp (key, "_id")) {
						bson_oid_copy (&value->value.v_oid, &user->oid);
					}

					else if (!strcmp (key, "role")) {
						bson_oid_copy (&value->value.v_oid, &user->role_oid);
					}

					else if (!strcmp (key, "name") && value->value.v_utf8.str) 
						user->name = str_new (value->value.v_utf8.str);

					else if (!strcmp (key, "email") && value->value.v_utf8.str) 
						user->email = str_new (value->value.v_utf8.str);

					else if (!strcmp (key, "username") && value->value.v_utf8.str) 
						user->username = str_new (value->value.v_utf8.str);

					else if (!strcmp (key, "password") && value->value.v_utf8.str) {
						user->password = str_new (value->value.v_utf8.str);
					}
				}
			}
		}
	}

	return user;

}

// get a user doc from the db by email
static const bson_t *user_find_by_email (const String *email, const DoubleList *select) {

	const bson_t *retval = NULL;

	bson_t *user_query = bson_new ();
	if (user_query) {
		bson_append_utf8 (user_query, "email", -1, email->str, email->len);
		retval = mongo_find_one (users_collection, user_query, select);
	}

	return retval;    

}

// gets a user from the db by its email
User *user_get_by_email (const String *email, const DoubleList *select) {

	User *user = NULL;

	if (email) {
		const bson_t *user_doc = user_find_by_email (email, select);
		if (user_doc) {
			user = user_doc_parse (user_doc);
			bson_destroy ((bson_t *) user_doc);
		}
	}

	return user;

}

// get a user doc from the db by username
static const bson_t *user_find_by_username (const String *username, const DoubleList *select) {

	const bson_t *retval = NULL;

	bson_t *user_query = bson_new ();
	if (user_query) {
		bson_append_utf8 (user_query, "username", -1, username->str, username->len);
		retval = mongo_find_one (users_collection, user_query, select);
	}

	return retval;    

}

// gets a user from the db by its username
User *user_get_by_username (const String *username, const DoubleList *select) {

	User *user = NULL;

	if (username) {
		const bson_t *user_doc = user_find_by_username (username, select);
		if (user_doc) {
			user = user_doc_parse (user_doc);
			bson_destroy ((bson_t *) user_doc);
		}
	}

	return user;

}

// {
//   "id": "5eb2b13f0051f70011e9d3af",
//   "name": "Erick Salas",
//   "username": "erick",
//   "role": "god",
//   "iat": 1596532954
// }
void *user_parse_from_json (void *user_json_ptr) {

	json_t *user_json = (json_t *) user_json_ptr;

	User *user = user_new ();
	if (user) {
		const char *email = NULL;
		const char *id = NULL;
		const char *name = NULL;
		const char *username = NULL;
		const char *role = NULL;

		if (!json_unpack (
			user_json,
			"{s:s, s:i, s:s, s:s, s:s, s:s}",
			"email", email,
			"iat", user->iat,
			"id", id,
			"name", name,
			"role", role,
			"username", username
		)) {
			user->id = str_new (id);
			user->name = str_new (name);
			user->username = str_new (username);
			user->email = str_new (email);
			user->role = str_new (role);

			user_print (user);
		}

		else {
			cerver_log_error ("user_parse_from_json () - json_unpack () has failed!");

			user_delete (user);
			user = NULL;
		}
	}

	return user;

}

bson_t *user_bson_create (User *user) {

	bson_t *doc = NULL;

	if (user) {
		doc = bson_new ();
		if (doc) {
			bson_oid_init (&user->oid, NULL);
			bson_append_oid (doc, "_id", -1, &user->oid);

			if (user->name) bson_append_utf8 (doc, "name", -1, user->name->str, user->name->len);
			if (user->username) bson_append_utf8 (doc, "username", -1, user->username->str, user->username->len);
			if (user->email) bson_append_utf8 (doc, "email", -1, user->email->str, user->email->len);
			if (user->password) bson_append_utf8 (doc, "password", -1, user->password->str, user->password->len);

			bson_append_oid (doc, "role", -1, &user->role_oid);
		}
	}

	return doc;

}