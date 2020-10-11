#ifndef _MODELS_USER_H_
#define _MODELS_USER_H_

#include <time.h>

#include <bson/bson.h>

#include <cerver/types/string.h>

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

extern int user_comparator (const void *a, const void *b);

extern void user_print (User *user);

// gets a user from the db by its email
extern User *user_get_by_email (const String *email);

// gets a user from the db by its username
extern User *user_get_by_username (const String *username);

extern void *user_parse_from_json (void *user_json_ptr);

#endif