#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cerver/types/string.h>

#include <cerver/collections/dlist.h>

#include <cerver/http/http.h>
#include <cerver/http/request.h>
#include <cerver/http/response.h>
#include <cerver/http/json/json.h>

#include <cerver/utils/utils.h>
#include <cerver/utils/log.h>

#include "mongo.h"
#include "pocket.h"

#include "controllers/roles.h"
#include "controllers/users.h"

#include "models/user.h"

// GET /api/users
void users_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	(void) http_response_send (users_works, http_receive);

}

// generate and send back token
static void users_generate_and_send_token (
	const HttpReceive *http_receive,
	const User *user, const String *role_name
) {

	bson_oid_to_string (&user->oid, (char *) user->id);

	DoubleList *payload = dlist_init (key_value_pair_delete, NULL);
	(void) dlist_insert_at_end_unsafe (payload, key_value_pair_create ("email", user->email));
	(void) dlist_insert_at_end_unsafe (payload, key_value_pair_create ("id", user->id));
	(void) dlist_insert_at_end_unsafe (payload, key_value_pair_create ("name", user->name));
	(void) dlist_insert_at_end_unsafe (payload, key_value_pair_create ("role", role_name->str));
	(void) dlist_insert_at_end_unsafe (payload, key_value_pair_create ("username", user->username));

	// generate & send back auth token
	char *token = http_cerver_auth_generate_jwt (
		(HttpCerver *) http_receive->cr->cerver->cerver_data, payload
	);

	if (token) {
		char *bearer = c_string_create ("Bearer %s", token);
		if (bearer) {
			char *json = c_string_create ("{\"token\": \"%s\"}", bearer);
			if (json) {
				HttpResponse *res = http_response_create (200, json, strlen (json));
				if (res) {
					http_response_compile (res);
					// http_response_print (res);
					(void) http_response_send (res, http_receive);
					http_respponse_delete (res);
				}

				free (json);
			}

			else {
				(void) http_response_send (server_error, http_receive);
			}

			free (bearer);
		}

		else {
			(void) http_response_send (server_error, http_receive);
		}

		free (token);
	}

	else {
		(void) http_response_send (server_error, http_receive);
	}

	dlist_delete (payload);

}

static u8 users_register_handler_save_user (
	const HttpReceive *http_receive,
	User *user
) {

	u8 retval = 1;

	if (!mongo_insert_one (
		users_collection,
		user_bson_create (user)
	)) {
		retval = 0;
	}

	else {
		cerver_log_error ("Failed to save new user!");
		(void) http_response_send (server_error, http_receive);
	}

	return retval;

}

// POST /api/users/register
void users_register_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	// get the user values from the request
	const String *name = http_request_body_get_value (request, "name");
	const String *username = http_request_body_get_value (request, "username");
	const String *email = http_request_body_get_value (request, "email");
	const String *password = http_request_body_get_value (request, "password");
	const String *confirm = http_request_body_get_value (request, "confirm");

	if (name && username && email && password && confirm) {
		if (!pocket_user_check_by_email (http_receive, email)) {
			User *user = pocket_user_create (
				name->str,
				username->str,
				email->str,
				password->str,
				&common_role->oid
			);
			if (user) {
				if (!users_register_handler_save_user (http_receive, user)) {
					cerver_log_success ("User %s has created an account!", email->str);

					// return token upon success
					users_generate_and_send_token (http_receive, user, common_role->name);
				}

				pocket_user_delete (user);
			}

			else {
				#ifdef POCKET_DEBUG
				cerver_log_error ("Failed to create user!");
				#endif
				(void) http_response_send (server_error, http_receive);
			}
		}
	}

	else {
		#ifdef POCKET_DEBUG
		cerver_log_warning ("Missing user values!");
		#endif
		(void) http_response_send (missing_user_values, http_receive);
	}

}

// POST /api/users/login
void users_login_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

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
				users_generate_and_send_token (
					http_receive,
					user, pocket_roles_get_by_oid (&user->role_oid)
				);
			}

			else {
				#ifdef POCKET_DEBUG
				cerver_log_error ("User %s login -> wrong password", oid_buffer);
				#endif

				(void) http_response_send (wrong_password, http_receive);
			}

			pocket_user_delete (user);
		}

		else {
			#ifdef POCKET_DEBUG
			cerver_log_warning ("User not found!");
			#endif
			(void) http_response_send (user_not_found, http_receive);
		}
	}

	else {
		#ifdef POCKET_DEBUG
		cerver_log_warning ("Missing user values!");
		#endif
		(void) http_response_send (missing_user_values, http_receive);
	}

}