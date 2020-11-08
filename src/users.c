#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cerver/types/string.h>

#include <cerver/collections/dlist.h>
#include <cerver/collections/pool.h>

#include <cerver/handler.h>
#include <cerver/version.h>

#include <cerver/http/http.h>
#include <cerver/http/request.h>
#include <cerver/http/response.h>
#include <cerver/http/json/json.h>

#include <cerver/utils/utils.h>
#include <cerver/utils/log.h>

#include "mongo.h"
#include "roles.h"
#include "users.h"

#include "models/user.h"

#pragma region main

static Pool *users_pool = NULL;

static const bson_t *user_login_query_opts = NULL;
static DoubleList *user_login_select = NULL;

const bson_t *user_transactions_query_opts = NULL;
DoubleList *user_transactions_select = NULL;

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

static unsigned int pocket_users_init_login_select (void) {

	unsigned int retval = 1;

	user_login_select = dlist_init (str_delete, str_comparator);
	dlist_insert_after (user_login_select, dlist_end (user_login_select), str_new ("name"));
	dlist_insert_after (user_login_select, dlist_end (user_login_select), str_new ("username"));
	dlist_insert_after (user_login_select, dlist_end (user_login_select), str_new ("email"));
	dlist_insert_after (user_login_select, dlist_end (user_login_select), str_new ("password"));
	dlist_insert_after (user_login_select, dlist_end (user_login_select), str_new ("role"));

	user_login_query_opts = mongo_find_generate_opts (user_login_select);

	user_transactions_select = dlist_init (str_delete, str_comparator);
	dlist_insert_after (user_transactions_select, dlist_end (user_login_select), str_new ("transCount"));
	dlist_insert_after (user_transactions_select, dlist_end (user_login_select), str_new ("transactions"));

	user_transactions_query_opts = mongo_find_generate_opts (user_transactions_select);

	if (user_login_query_opts && user_transactions_query_opts) retval = 0;

	return retval;

}

unsigned int pocket_users_init (void) {

	unsigned int errors = 0;

	errors |= pocket_users_init_pool ();

	errors |= pocket_users_init_login_select ();

	return errors;

}

void pocket_users_end (void) {

	dlist_delete (user_login_select);
	bson_destroy ((bson_t *) user_login_query_opts);

	dlist_delete (user_transactions_select);
	bson_destroy ((bson_t *) user_transactions_query_opts);

	pool_delete (users_pool);
	users_pool = NULL;

}

static User *pocket_user_create (
	const char *name,
	const char *username,
	const char *email,
	const char *password,
	const bson_oid_t *role_oid
) {

	User *user = (User *) pool_pop (users_pool);
	if (user) {
		bson_oid_init (&user->oid, NULL);

		strncpy (user->name, name, USER_NAME_LEN);
		strncpy (user->username, username, USER_USERNAME_LEN);
		strncpy (user->email, email, USER_EMAIL_LEN);
		strncpy (user->password, password, USER_PASSWORD_LEN);

		bson_oid_copy (role_oid, &user->role_oid);
	}

	return user;

}

static User *pocket_user_get_by_email (const String *email) {

	User *user = NULL;
	if (email) {
		user = (User *) pool_pop (users_pool);
		if (user) {
			if (user_get_by_email (user, email, user_login_query_opts)) {
				(void) pool_push (users_pool, user);
				user = NULL;
			}
		}
	}

	return user;

}

// {
//   "email": "erick.salas@ermiry.com",
//   "iat": 1596532954
//   "id": "5eb2b13f0051f70011e9d3af",
//   "name": "Erick Salas",
//   "role": "god",
//   "username": "erick",
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
			strncpy (user->email, email, USER_EMAIL_LEN);
			strncpy (user->id, id, USER_ID_LEN);
			strncpy (user->name, name, USER_NAME_LEN);
			strncpy (user->role, role, USER_ROLE_LEN);
			strncpy (user->username, username, USER_USERNAME_LEN);

			user_print (user);
		}

		else {
			cerver_log_error ("user_parse_from_json () - json_unpack () has failed!");

			pool_push (users_pool, user);
			user = NULL;
		}
	}

	return user;

}

void pocket_user_delete (void *user_ptr) {

	memset (user_ptr, 0, sizeof (User));
	pool_push (users_pool, user_ptr);

}

#pragma endregion

#pragma region routes

// GET api/users/
void users_handler (CerverReceive *cr, HttpRequest *request) {

	http_response_json_msg_send (cr, 200, "Users works!");

}

// generate and send back token
static void users_generate_and_send_token (CerverReceive *cr, const User *user, const String *role_name) {

	bson_oid_to_string (&user->oid, (char *) user->id);

	DoubleList *payload = dlist_init (key_value_pair_delete, NULL);
	dlist_insert_after (payload, dlist_end (payload), key_value_pair_create ("email", user->email));
	dlist_insert_after (payload, dlist_end (payload), key_value_pair_create ("id", user->id));
	dlist_insert_after (payload, dlist_end (payload), key_value_pair_create ("name", user->name));
	dlist_insert_after (payload, dlist_end (payload), key_value_pair_create ("role", role_name->str));
	dlist_insert_after (payload, dlist_end (payload), key_value_pair_create ("username", user->username));

	// generate & send back auth token
	char *token = http_cerver_auth_generate_jwt ((HttpCerver *) cr->cerver->cerver_data, payload);
	if (token) {
		char *bearer = c_string_create ("Bearer %s", token);
		if (bearer) {
			char *json = c_string_create ("{\"token\": \"%s\"}", bearer);
			if (json) {
				HttpResponse *res = http_response_create (200, json, strlen (json));
				if (res) {
					http_response_compile (res);
					// http_response_print (res);
					http_response_send (res, cr->cerver, cr->connection);
					http_respponse_delete (res);
				}

				free (json);
			}

			else {
				http_response_json_error_send (cr, 500, "Internal error!");
			}

			free (bearer);
		}

		else {
			http_response_json_error_send (cr, 500, "Internal error!");
		}

		free (token);
	}

	else {
		http_response_json_error_send (cr, 500, "Internal error!");
	}

	dlist_delete (payload);

}

static u8 users_register_handler_save_user (CerverReceive *cr, User *user) {

	u8 retval = 1;

	if (!mongo_insert_one (
		users_collection,
		user_bson_create (user)
	)) {
		retval = 0;
	}

	else {
		cerver_log_error ("Failed to save new user!");
		http_response_json_error_send (cr, 500, "Internal error!");
	}

	return retval;

}

// POST api/users/register
void users_register_handler (CerverReceive *cr, HttpRequest *request) {

	// get the user values from the request
	const String *name = http_request_body_get_value (request, "name");
	const String *username = http_request_body_get_value (request, "username");
	const String *email = http_request_body_get_value (request, "email");
	const String *password = http_request_body_get_value (request, "password");
	const String *confirm = http_request_body_get_value (request, "confirm");

	if (name && username && email && password && confirm) {
		User *user = pocket_user_create (
			name->str,
			username->str,
			email->str,
			password->str,
			&common_role->oid
		);
		if (user) {
			if (!users_register_handler_save_user (cr, user)) {
				cerver_log_success ("User %s has created an account!", email->str);

				// return token upon success
				users_generate_and_send_token (cr, user, common_role->name);
			}

			pocket_user_delete (user);
		}

		else {
			#ifdef POCKET_DEBUG
			cerver_log_error ("Failed to create user!");
			#endif
			http_response_json_error_send (cr, 500, "Internal error!");
		}
	}

	else {
		#ifdef POCKET_DEBUG
		cerver_log_warning ("Missing user values!");
		#endif
		http_response_json_error_send (cr, 400, "Missing user values!");
	}

}

// POST api/users/login
void users_login_handler (CerverReceive *cr, HttpRequest *request) {

	// get the user values from the request
	// const String *username = http_request_body_get_value (request, "username");
	const String *email = http_request_body_get_value (request, "email");
	const String *password = http_request_body_get_value (request, "password");

	if (email && password) {
		User *user = pocket_user_get_by_email (email);
		if (user) {
			#ifdef POCKET_DEBUG
			char oid_buffer[32] = { 0 };
			bson_oid_to_string (&user->oid, oid_buffer);
			#endif

			if (!strcmp (user->password, password->str)) {
				#ifdef POCKET_DEBUG
				cerver_log_success ("User %s login -> success", oid_buffer);
				#endif

				// generate and send token back to the user
				users_generate_and_send_token (cr, user, pocket_roles_get_by_oid (&user->role_oid));
			}

			else {
				#ifdef POCKET_DEBUG
				cerver_log_error ("User %s login -> wrong password", oid_buffer);
				#endif

				http_response_json_error_send (cr, 400, "Password is incorrect!");
			}

			pocket_user_delete (user);
		}

		else {
			#ifdef POCKET_DEBUG
			cerver_log_warning ("User not found!");
			#endif
			http_response_json_error_send (cr, 404, "User not found!");
		}
	}

	else {
		#ifdef POCKET_DEBUG
		cerver_log_warning ("Missing user values!");
		#endif
		http_response_json_error_send (cr, 400, "Missing user values!");
	}

}

#pragma endregion