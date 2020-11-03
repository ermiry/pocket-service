#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cerver/types/string.h>

#include <cerver/collections/dlist.h>

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

#include "models/user.h"

#pragma region main

static DoubleList *user_login_select = NULL;

unsigned int pocket_users_init (void) {

	user_login_select = dlist_init (str_delete, str_comparator);
	dlist_insert_after (user_login_select, dlist_end (user_login_select), str_new ("name"));
	dlist_insert_after (user_login_select, dlist_end (user_login_select), str_new ("username"));
	dlist_insert_after (user_login_select, dlist_end (user_login_select), str_new ("password"));
	dlist_insert_after (user_login_select, dlist_end (user_login_select), str_new ("role"));

	return 0;

}

void pocket_users_end (void) {

	dlist_delete (user_login_select);

}

#pragma endregion

#pragma region routes

// GET api/users/
void users_handler (CerverReceive *cr, HttpRequest *request) {

	http_response_json_msg_send (cr, 200, "Users works!");

}

// generate and send back token
static void users_generate_and_send_token (CerverReceive *cr, const User *user, const String *role_name) {

	char buffer[32] = { 0 };
	bson_oid_to_string (&user->oid, buffer);
	String *id = str_new (buffer);

	DoubleList *payload = dlist_init (key_value_pair_delete, NULL);
	dlist_insert_after (payload, dlist_end (payload), key_value_pair_create ("email", user->email->str));
	dlist_insert_after (payload, dlist_end (payload), key_value_pair_create ("id", id->str));
	dlist_insert_after (payload, dlist_end (payload), key_value_pair_create ("name", user->name->str));
	dlist_insert_after (payload, dlist_end (payload), key_value_pair_create ("role", role_name->str));
	dlist_insert_after (payload, dlist_end (payload), key_value_pair_create ("username", user->username->str));

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
		User *user = user_create (
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

			user_delete (user);
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
		User *user = user_get_by_email (email, user_login_select);
		if (user) {
			#ifdef POCKET_DEBUG
			char oid_buffer[32] = { 0 };
			bson_oid_to_string (&user->oid, oid_buffer);
			#endif

			if (!strcmp (user->password->str, password->str)) {
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

			user_delete (user);
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