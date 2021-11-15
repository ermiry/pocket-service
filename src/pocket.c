#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <cerver/types/types.h>
#include <cerver/types/string.h>

#include <cerver/handler.h>

#include <cerver/utils/log.h>
#include <cerver/utils/utils.h>

#include <cmongo/mongo.h>

#include <credis/redis.h>

#include "pocket.h"
#include "runtime.h"
#include "version.h"

#include "models/action.h"
#include "models/category.h"
#include "models/place.h"
#include "models/role.h"
#include "models/user.h"

#include "controllers/categories.h"
#include "controllers/places.h"
#include "controllers/roles.h"
#include "controllers/service.h"
#include "controllers/transactions.h"
#include "controllers/users.h"

RuntimeType RUNTIME = RUNTIME_TYPE_NONE;

unsigned int PORT = CERVER_DEFAULT_PORT;

unsigned int CERVER_RECEIVE_BUFFER_SIZE = CERVER_DEFAULT_RECEIVE_BUFFER_SIZE;
unsigned int CERVER_TH_THREADS = CERVER_DEFAULT_POOL_THREADS;
unsigned int CERVER_CONNECTION_QUEUE = CERVER_DEFAULT_CONNECTION_QUEUE;

static char MONGO_URI[MONGO_URI_SIZE] = { 0 };
static char MONGO_APP_NAME[MONGO_APP_NAME_SIZE] = { 0 };
static char MONGO_DB[MONGO_DB_SIZE] = { 0 };

static char REAL_PUB_KEY[PUB_KEY_SIZE] = { 0 };
const char *PUB_KEY = REAL_PUB_KEY;

bool CONNECT_TO_REDIS = false;
static char REAL_REDIS_HOSTNAME[REDIS_HOSTNAME_SIZE] = { 0 };
const char *REDIS_HOSTNAME = REAL_REDIS_HOSTNAME;

static void pocket_env_get_runtime (void) {

	char *runtime_env = getenv ("RUNTIME");
	if (runtime_env) {
		RUNTIME = runtime_from_string (runtime_env);
		cerver_log_success (
			"RUNTIME -> %s", runtime_to_string (RUNTIME)
		);
	}

	else {
		cerver_log_warning ("Failed to get RUNTIME from env!");
	}

}

static unsigned int pocket_env_get_port (void) {

	unsigned int retval = 1;

	char *port_env = getenv ("PORT");
	if (port_env) {
		PORT = (unsigned int) atoi (port_env);
		cerver_log_success (
			"PORT -> %u", PORT
		);

		retval = 0;
	}

	else {
		cerver_log_error ("Failed to get PORT from env!");
	}

	return retval;

}

static void pocket_env_get_cerver_receive_buffer_size (void) {

	char *buffer_size = getenv ("CERVER_RECEIVE_BUFFER_SIZE");
	if (buffer_size) {
		CERVER_RECEIVE_BUFFER_SIZE = (unsigned int) atoi (buffer_size);
		cerver_log_success (
			"CERVER_RECEIVE_BUFFER_SIZE -> %u", CERVER_RECEIVE_BUFFER_SIZE
		);
	}

	else {
		cerver_log_warning (
			"Failed to get CERVER_RECEIVE_BUFFER_SIZE from env - using default %u!",
			CERVER_RECEIVE_BUFFER_SIZE
		);
	}
}

static void pocket_env_get_cerver_th_threads (void) {

	char *th_threads = getenv ("CERVER_TH_THREADS");
	if (th_threads) {
		CERVER_TH_THREADS = (unsigned int) atoi (th_threads);
		cerver_log_success ("CERVER_TH_THREADS -> %u", CERVER_TH_THREADS);
	}

	else {
		cerver_log_warning (
			"Failed to get CERVER_TH_THREADS from env - using default %u!",
			CERVER_TH_THREADS
		);
	}

}

static void pocket_env_get_cerver_connection_queue (void) {

	char *connection_queue = getenv ("CERVER_CONNECTION_QUEUE");
	if (connection_queue) {
		CERVER_CONNECTION_QUEUE = (unsigned int) atoi (connection_queue);
		cerver_log_success ("CERVER_CONNECTION_QUEUE -> %u", CERVER_CONNECTION_QUEUE);
	}

	else {
		cerver_log_warning (
			"Failed to get CERVER_CONNECTION_QUEUE from env - using default %u!",
			CERVER_CONNECTION_QUEUE
		);
	}

}

static unsigned int pocket_env_get_mongo_app_name (void) {

	unsigned int retval = 1;

	char *mongo_app_name_env = getenv ("MONGO_APP_NAME");
	if (mongo_app_name_env) {
		(void) strncpy (
			MONGO_APP_NAME,
			mongo_app_name_env,
			MONGO_APP_NAME_SIZE - 1
		);

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
		(void) strncpy (
			MONGO_DB,
			mongo_db_env,
			MONGO_DB_SIZE - 1
		);

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
		(void) strncpy (
			MONGO_URI,
			mongo_uri_env,
			MONGO_URI_SIZE - 1
		);

		retval = 0;
	}

	else {
		cerver_log_error ("Failed to get MONGO_URI from env!");
	}

	return retval;

}

static unsigned int pocket_env_get_public_key (void) {

	unsigned int retval = 1;

	char *pub_key_env = getenv ("PUB_KEY");
	if (pub_key_env) {
		(void) strncpy (
			REAL_PUB_KEY,
			pub_key_env,
			PUB_KEY_SIZE - 1
		);

		retval = 0;
	}

	else {
		cerver_log_error ("Failed to get PUB_KEY from env!");
	}

	return retval;

}

static void pocket_env_get_connect_to_redis (void) {

	char *connect_to_redis = getenv ("CONNECT_TO_REDIS");
	if (connect_to_redis) {
		if (!strcasecmp (connect_to_redis, "TRUE")) {
			CONNECT_TO_REDIS = true;
			cerver_log_success ("CONNECT_TO_REDIS -> TRUE");
		}

		else {
			CONNECT_TO_REDIS = false;
			cerver_log_success ("CONNECT_TO_REDIS -> FALSE");
		}
	}

	else {
		cerver_log_warning (
			"Failed to get CONNECT_TO_REDIS from env - using default FALSE!"
		);
	}

}

static unsigned int pocket_env_get_redis_hostname (void) {

	unsigned int retval = 1;

	char *pocket_REDIS_HOSTNAME_env = getenv ("REDIS_HOSTNAME");
	if (pocket_REDIS_HOSTNAME_env) {
		(void) strncpy (
			REAL_REDIS_HOSTNAME,
			pocket_REDIS_HOSTNAME_env,
			REDIS_HOSTNAME_SIZE - 1
		);

		retval = 0;
	}

	else {
		cerver_log_error ("Failed to get REDIS_HOSTNAME from env!");
	}

	return retval;

}

static unsigned int pocket_init_env (void) {

	unsigned int errors = 0;

	pocket_env_get_runtime ();

	errors |= pocket_env_get_port ();

	pocket_env_get_cerver_receive_buffer_size ();

	pocket_env_get_cerver_th_threads ();

	pocket_env_get_cerver_connection_queue ();

	errors |= pocket_env_get_mongo_app_name ();

	errors |= pocket_env_get_mongo_db ();

	errors |= pocket_env_get_mongo_uri ();

	errors |= pocket_env_get_public_key ();

	pocket_env_get_connect_to_redis ();

	if (CONNECT_TO_REDIS) {
		errors |= pocket_env_get_redis_hostname ();
	}

	return errors;

}

static unsigned int pocket_mongo_connect (void) {

	unsigned int errors = 0;

	bool connected_to_mongo = false;

	mongo_set_uri (MONGO_URI);
	mongo_set_app_name (MONGO_APP_NAME);
	mongo_set_db_name (MONGO_DB);

	if (!mongo_connect ()) {
		// test mongo connection
		if (!mongo_ping_db ()) {
			cerver_log_success ("Connected to Mongo DB!");

			errors |= actions_model_init ();

			errors |= categories_model_init ();

			errors |= places_model_init ();

			errors |= roles_model_init ();

			errors |= transactions_model_init ();

			errors |= users_model_init ();

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

static unsigned int pocket_redis_init (void) {

	unsigned int errors = 0;

	if (CONNECT_TO_REDIS) {
		unsigned int result = 1;

		char *hostname = network_hostname_to_ip (REDIS_HOSTNAME);
		if (hostname) {
			credis_set_hostname (hostname);

			if (!credis_init ()) {
				result = credis_ping_db ();
			}

			free (hostname);
		}

		else {
			cerver_log_error ("Failed to get REDIS_HOSTNAME ip address!");
		}

		errors = result;
	}

	return errors;

}

static unsigned int pocket_init_internal (void) {

	unsigned int errors = 0;

	errors |= pocket_mongo_init ();

	errors |= pocket_service_init ();

	errors |= pocket_users_init ();

	errors |= pocket_categories_init ();

	errors |= pocket_places_init ();

	errors |= pocket_trans_init ();

	return errors;

}

// inits pocket main values
unsigned int pocket_init (void) {

	unsigned int retval = 1;

	if (!pocket_init_env ()) {
		if (!pocket_mongo_init ()) {
			if (!pocket_redis_init ()) {
				retval = pocket_init_internal ();
			}
		}
	}

	return retval;

}

static unsigned int pocket_mongo_end (void) {

	if (mongo_get_status () == MONGO_STATUS_CONNECTED) {
		actions_model_end ();

		categories_model_end ();

		places_model_end ();

		roles_model_end ();

		transactions_model_end ();

		users_model_end ();

		mongo_disconnect ();
	}

	return 0;

}

// ends pocket main values
unsigned int pocket_end (void) {

	unsigned int errors = 0;

	errors |= pocket_mongo_end ();

	if (CONNECT_TO_REDIS) {
		(void) credis_end ();
	}

	pocket_roles_end ();

	pocket_users_end ();

	pocket_categories_end ();

	pocket_places_end ();

	pocket_trans_end ();

	pocket_service_end ();

	return errors;

}
