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

#include "things.h"
#include "mongo.h"
#include "roles.h"
#include "version.h"

#include "models/action.h"
#include "models/role.h"
#include "models/user.h"

#pragma region main

const String *PORT = NULL;
static const String *MONGO_URI = NULL;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

static unsigned int things_env_get_port (void) {
	
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

static unsigned int things_env_get_mongo_uri (void) {

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

#pragma GCC diagnostic pop

static unsigned int things_init_env (void) {

	unsigned int errors = 0;

	errors |= things_env_get_port ();

	errors |= things_env_get_mongo_uri ();

	return errors;

}

static unsigned int things_mongo_connect (void) {

	unsigned int errors = 0;

	bool connected_to_mongo = false;

	mongo_set_uri (MONGO_URI->str);
	mongo_set_app_name ("things");
	mongo_set_db_name ("ermiry");

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

static unsigned int things_mongo_init (void) {

	unsigned int retval = 1;

	if (!things_mongo_connect ()) {
		if (!things_roles_init ()) {
			retval = 0;
		}

		else {
			cerver_log_error ("Failed to get roles from db!");
		}
	}

	return retval;

}

// inits things main values
unsigned int things_init (void) {

	unsigned int errors = 0;

	if (!things_init_env ()) {
		errors |= things_mongo_init ();
	}

	return errors;  

}

static unsigned int things_mongo_end (void) {

	if (mongo_get_status () == MONGO_STATUS_CONNECTED) {
		actions_collection_close ();

		roles_collection_close ();

		users_collection_close ();

		mongo_disconnect ();
	}

	return 0;

}

// ends things main values
unsigned int things_end (void) {

	unsigned int errors = 0;

	errors |= things_mongo_end ();

	things_roles_end ();

	return errors;

}

#pragma endregion

#pragma region routes

// GET api/things/
void things_handler (CerverReceive *cr, HttpRequest *request) {

	http_response_json_msg_send (cr, 200, "Things works!");

}

// GET api/things/version
void things_version_handler (CerverReceive *cr, HttpRequest *request) {

	char *status = c_string_create ("%s - %s", THINGS_VERSION_NAME, THINGS_VERSION_DATE);
	if (status) {
		http_response_json_msg_send (cr, 200, status);
		free (status);
	}

}

// GET api/things/auth
void things_auth_handler (CerverReceive *cr, HttpRequest *request) {

	http_response_json_msg_send (cr, 200, "Things auth!");

}

#pragma endregion