#ifndef _MODELS_USER_H_
#define _MODELS_USER_H_

#include <time.h>

#include <mongoc/mongoc.h>
#include <bson/bson.h>

#include <cerver/types/string.h>

extern mongoc_collection_t *users_collection;

// opens handle to user collection
extern unsigned int users_collection_get (void);

extern void users_collection_close (void);

typedef struct User {

	String *id;
	bson_oid_t oid;

	String *name;
	String *username;
	String *email;
	String *password;

	String *role;
	bson_oid_t role_oid;

	time_t iat;

} User;

extern User *user_new (void);

extern void user_delete (void *user_ptr);

extern User *user_create (
	const char *name,
	const char *username,
	const char *email,
	const char *password,
	const bson_oid_t *role_oid
);

extern int user_comparator (const void *a, const void *b);

extern void user_print (User *user);

// gets a user from the db by its email
extern User *user_get_by_email (const String *email, const DoubleList *select);

// gets a user from the db by its username
extern User *user_get_by_username (const String *username, const DoubleList *select);

extern void *user_parse_from_json (void *user_json_ptr);

extern bson_t *user_bson_create (User *user);

#endif