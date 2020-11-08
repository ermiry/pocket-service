#ifndef _MODELS_USER_H_
#define _MODELS_USER_H_

#include <time.h>

#include <mongoc/mongoc.h>
#include <bson/bson.h>

#include <cerver/types/types.h>
#include <cerver/types/string.h>

#include <cerver/collections/dlist.h>

#define USER_ID_LEN				32
#define USER_EMAIL_LEN			128
#define USER_NAME_LEN			128
#define USER_USERNAME_LEN		128
#define USER_PASSWORD_LEN		128
#define USER_ROLE_LEN			64

extern mongoc_collection_t *users_collection;

// opens handle to user collection
extern unsigned int users_collection_get (void);

extern void users_collection_close (void);

typedef struct User {

	char id[USER_ID_LEN];
	bson_oid_t oid;

	char email[USER_EMAIL_LEN];
	char name[USER_NAME_LEN];
	char username[USER_USERNAME_LEN];
	char password[USER_PASSWORD_LEN];

	char role[USER_ROLE_LEN];
	bson_oid_t role_oid;

	time_t iat;

	int trans_count;
	DoubleList *trans_docs;
	DoubleList *transactions;

} User;

extern void *user_new (void);

extern void user_delete (void *user_ptr);

extern void user_print (User *user);

extern bson_t *user_query_id (const char *id);

// gets a user from the db by its email
extern u8 user_get_by_email (
	User *user, const String *email, const bson_t *query_opts
);

// gets a user from the db by its username
extern u8 user_get_by_username (
	User *user, const String *username, const bson_t *query_opts
);

extern bson_t *user_bson_create (User *user);

// pushes a new trans oid to user's transactions array
// and adds one to user's transactions count
extern bson_t *user_create_update_pocket_transactions (const bson_oid_t *trans_oid);

#endif