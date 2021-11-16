#ifndef _MODELS_USER_H_
#define _MODELS_USER_H_

#include <stdbool.h>

#include <time.h>

#include <bson/bson.h>
#include <mongoc/mongoc.h>

#define USERS_COLL_NAME			"users"

#define USER_ID_SIZE			32
#define USER_EMAIL_SIZE			128
#define USER_NAME_SIZE			128
#define USER_USERNAME_SIZE		128
#define USER_ROLE_SIZE			64

extern unsigned int users_model_init (void);

extern void users_model_end (void);

typedef struct User {

	// user's unique id
	char id[USER_ID_SIZE];
	bson_oid_t oid;

	// main user values
	char email[USER_EMAIL_SIZE];
	char name[USER_NAME_SIZE];
	char username[USER_USERNAME_SIZE];

	char role[USER_ROLE_SIZE];
	bson_oid_t role_oid;

	// used to validate JWT expiration
	time_t iat;

} User;

extern void *user_new (void);

extern void user_delete (void *user_ptr);

extern void user_print (const User *user);

extern unsigned int user_get_by_id (
	User *user, const char *id, const bson_t *query_opts
);

extern bool user_check_by_email (const char *email);

// gets a user from the db by its email
extern unsigned int user_get_by_email (
	User *user, const char *email, const bson_t *query_opts
);

#endif