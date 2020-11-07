#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cerver/types/types.h>
#include <cerver/types/string.h>

#include <cerver/handler.h>

#include <cerver/http/http.h>
#include <cerver/http/route.h>
#include <cerver/http/request.h>
#include <cerver/http/response.h>
#include <cerver/http/json/json.h>

#include <cerver/utils/utils.h>
#include <cerver/utils/log.h>

#include "pocket.h"
#include "mongo.h"
#include "roles.h"
#include "users.h"
#include "version.h"

#include "models/action.h"
#include "models/role.h"
#include "models/user.h"

#pragma region main

const String *PORT = NULL;

static const String *MONGO_URI = NULL;
static const String *MONGO_APP_NAME = NULL;
static const String *MONGO_DB = NULL;

unsigned int CERVER_RECEIVE_BUFFER_SIZE = 4096;
unsigned int CERVER_TH_THREADS = 4;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

static unsigned int pocket_env_get_port (void) {
	
	unsigned int retval = 1;

	char *port_env = getenv ("PORT");
	if (port_env) {
		PORT = str_new (port_env);

		retval = 0;
	}

	else {
		cerver_log_error ("Failed to get PORT from env!");
	}

	return retval;

}

static unsigned int pocket_env_get_mongo_app_name (void) {

	unsigned int retval = 1;

	char *mongo_app_name_env = getenv ("MONGO_APP_NAME");
	if (mongo_app_name_env) {
		MONGO_APP_NAME = str_new (mongo_app_name_env);

		retval = 0;
	}

	else {
		cerver_log_error ("Failed to get MONGO_APP_NAME from env!");
	}

	return retval;

}

static unsigned int pocket_env_get_mongo_db (void) {

	unsigned int retval = 1;

	char *mongo_db_env = getenv ("MONGO_DB");
	if (mongo_db_env) {
		MONGO_DB = str_new (mongo_db_env);

		retval = 0;
	}

	else {
		cerver_log_error ("Failed to get MONGO_DB from env!");
	}

	return retval;

}

static unsigned int pocket_env_get_mongo_uri (void) {

	unsigned int retval = 1;

	char *mongo_uri_env = getenv ("MONGO_URI");
	if (mongo_uri_env) {
		MONGO_URI = str_new (mongo_uri_env);

		retval = 0;
	}

	else {
		cerver_log_error ("Failed to get MONGO_URI from env!");
	}

	return retval;

}

static void gepp_env_get_cerver_receive_buffer_size (void) {

	char *buffer_size = getenv ("CERVER_RECEIVE_BUFFER_SIZE");
	if (buffer_size) {
		CERVER_RECEIVE_BUFFER_SIZE = (unsigned int) atoi (buffer_size);
		cerver_log_success (
			"CERVER_RECEIVE_BUFFER_SIZE -> %d\n", CERVER_RECEIVE_BUFFER_SIZE
		);
	}

	else {
		cerver_log_warning (
			"Failed to get CERVER_RECEIVE_BUFFER_SIZE from env - using default %d!",
			CERVER_RECEIVE_BUFFER_SIZE
		);
	}
}

static void gepp_env_get_cerver_th_threads (void) {

	char *th_threads = getenv ("CERVER_TH_THREADS");
	if (th_threads) {
		CERVER_TH_THREADS = (unsigned int) atoi (th_threads);
		cerver_log_success ("CERVER_TH_THREADS -> %d\n", CERVER_TH_THREADS);
	}

	else {
		cerver_log_warning (
			"Failed to get CERVER_TH_THREADS from env - using default %d!",
			CERVER_TH_THREADS
		);
	}

}

#pragma GCC diagnostic pop

static unsigned int pocket_init_env (void) {

	unsigned int errors = 0;

	errors |= pocket_env_get_port ();

	errors |= pocket_env_get_mongo_uri ();

	errors |= pocket_env_get_mongo_app_name ();

	errors |= pocket_env_get_mongo_db ();

	return errors;

}

static unsigned int pocket_mongo_connect (void) {

	unsigned int errors = 0;

	bool connected_to_mongo = false;

	mongo_set_uri (MONGO_URI->str);
	mongo_set_app_name (MONGO_APP_NAME->str);
	mongo_set_db_name (MONGO_DB->str);

	if (!mongo_connect ()) {
		// test mongo connection
		if (!mongo_ping_db ()) {
			cerver_log_success ("Connected to Mongo DB!");

			// open handle to actions collection
			errors |= actions_collection_get ();

			// open handle to role collection
			errors |= roles_collection_get ();

			// open handle to user collection
			errors |= users_collection_get ();

			connected_to_mongo = true;
		}
	}

	if (!connected_to_mongo) {
		cerver_log_error ("Failed to connect to mongo!");
		errors |= 1;
	}

	return errors;

}

static unsigned int pocket_mongo_init (void) {

	unsigned int retval = 1;

	if (!pocket_mongo_connect ()) {
		if (!pocket_roles_init ()) {
			retval = 0;
		}

		else {
			cerver_log_error ("Failed to get roles from db!");
		}
	}

	return retval;

}

// inits pocket main values
unsigned int pocket_init (void) {

	unsigned int errors = 0;

	if (!pocket_init_env ()) {
		errors |= pocket_mongo_init ();

		errors |= pocket_users_init ();
	}

	return errors;  

}

static unsigned int pocket_mongo_end (void) {

	if (mongo_get_status () == MONGO_STATUS_CONNECTED) {
		actions_collection_close ();

		roles_collection_close ();

		users_collection_close ();

		mongo_disconnect ();
	}

	return 0;

}

// ends pocket main values
unsigned int pocket_end (void) {

	unsigned int errors = 0;

	errors |= pocket_mongo_end ();

	pocket_roles_end ();

	pocket_users_end ();

	str_delete ((String *) MONGO_URI);
	str_delete ((String *) MONGO_APP_NAME);
	str_delete ((String *) MONGO_DB);

	return errors;

}

#pragma endregion

#pragma region routes

// GET api/pocket/
void pocket_handler (CerverReceive *cr, HttpRequest *request) {

	http_response_json_msg_send (cr, 200, "Pocket works!");

}

// GET api/pocket/version
void pocket_version_handler (CerverReceive *cr, HttpRequest *request) {

	char *status = c_string_create ("%s - %s", POCKET_VERSION_NAME, POCKET_VERSION_DATE);
	if (status) {
		http_response_json_msg_send (cr, 200, status);
		free (status);
	}

}

// GET api/pocket/auth
void pocket_auth_handler (CerverReceive *cr, HttpRequest *request) {

	User *user = (User *) request->decoded_data;
	if (user) {
		#ifdef POCKET_DEBUG
		user_print (user);
		#endif

		http_response_json_msg_send (cr, 200, "Pocket auth!");
	}

	else {
		http_request_multi_part_discard_files (request);
		cerver_log_warning ("Bad user!");
		http_response_json_error_send (cr, 400, "Bad user!");
	}

}

// GET api/pocket/transactions
// get all the authenticated user's transactions
void pocket_transactions_handler (CerverReceive *cr, HttpRequest *request) {

}

#pragma endregion