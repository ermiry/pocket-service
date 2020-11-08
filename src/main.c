#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <signal.h>

#include <cerver/version.h>
#include <cerver/cerver.h>

#include <cerver/http/http.h>
#include <cerver/http/route.h>

#include <cerver/utils/utils.h>
#include <cerver/utils/log.h>

#include "handler.h"
#include "pocket.h"
#include "users.h"
#include "version.h"

static Cerver *pocket_api = NULL;

void end (int dummy) {
	
	if (pocket_api) {
		cerver_stats_print (pocket_api, false, false);
		printf ("\nHTTP Cerver stats:\n");
		http_cerver_all_stats_print ((HttpCerver *) pocket_api->cerver_data);
		printf ("\n");
		cerver_teardown (pocket_api);
	}

	pocket_end ();

	cerver_end ();

	exit (0);

}

static void pocket_set_pocket_routes (HttpCerver *http_cerver) {

	/* register top level route */
	// GET /api/pocket/
	HttpRoute *pocket_route = http_route_create (REQUEST_METHOD_GET, "api/pocket", pocket_handler);
	http_cerver_route_register (http_cerver, pocket_route);

	/* register pocket children routes */
	// GET api/pocket/version/
	HttpRoute *pocket_version_route = http_route_create (REQUEST_METHOD_GET, "version", pocket_version_handler);
	http_route_child_add (pocket_route, pocket_version_route);

	// GET api/pocket/auth/
	HttpRoute *pocket_auth_route = http_route_create (REQUEST_METHOD_GET, "auth", pocket_auth_handler);
	http_route_set_auth (pocket_auth_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (pocket_auth_route, pocket_user_parse_from_json, pocket_user_delete);
	http_route_child_add (pocket_route, pocket_auth_route);

	/*** transactions ***/

	// GET api/pocket/transactions
	HttpRoute *transactions_route = http_route_create (REQUEST_METHOD_GET, "transactions", pocket_transactions_handler);
	http_route_set_auth (transactions_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (transactions_route, pocket_user_parse_from_json, pocket_user_delete);
	http_route_child_add (pocket_route, transactions_route);

	// POST api/pocket/transactions
	http_route_set_handler (transactions_route, REQUEST_METHOD_POST, pocket_transaction_create_handler);

}

static void pocket_set_users_routes (HttpCerver *http_cerver) {

	/* register top level route */
	// GET /api/users/
	HttpRoute *users_route = http_route_create (REQUEST_METHOD_GET, "api/users", users_handler);
	http_cerver_route_register (http_cerver, users_route);

	/* register users children routes */
	// POST api/users/login
	HttpRoute *users_login_route = http_route_create (REQUEST_METHOD_POST, "login", users_login_handler);
	http_route_child_add (users_route, users_login_route);

	// POST api/users/register
	HttpRoute *users_register_route = http_route_create (REQUEST_METHOD_POST, "register", users_register_handler);
	http_route_child_add (users_route, users_register_route);

}

static void start (void) {

	pocket_api = cerver_create (CERVER_TYPE_WEB, "pocket-api", atoi (PORT->str), PROTOCOL_TCP, false, 10, 1000);
	if (pocket_api) {
		/*** cerver configuration ***/
		cerver_set_receive_buffer_size (pocket_api, CERVER_RECEIVE_BUFFER_SIZE);
		cerver_set_thpool_n_threads (pocket_api, CERVER_TH_THREADS);
		cerver_set_handler_type (pocket_api, CERVER_HANDLER_TYPE_THREADS);

		/*** web cerver configuration ***/
		HttpCerver *http_cerver = (HttpCerver *) pocket_api->cerver_data;

		http_cerver_auth_set_jwt_algorithm (http_cerver, JWT_ALG_RS256);
		http_cerver_auth_set_jwt_priv_key_filename (http_cerver, "keys/key.key");
		http_cerver_auth_set_jwt_pub_key_filename (http_cerver, "keys/key.pub");

		pocket_set_pocket_routes (http_cerver);
		pocket_set_users_routes (http_cerver);

		// add a catch all route
		http_cerver_set_catch_all_route (http_cerver, pocket_catch_all_handler);

		if (cerver_start (pocket_api)) {
			cerver_log_error (
				"Failed to start %s!",
				pocket_api->info->name->str
			);

			cerver_delete (pocket_api);
		}
	}

	else {
		cerver_log_error ("Failed to create cerver!");

		cerver_delete (pocket_api);
	}

}

int main (int argc, char const **argv) {

	srand (time (NULL));

	// register to the quit signal
	signal (SIGINT, end);
	signal (SIGTERM, end);

	// to prevent SIGPIPE when writting to socket
	signal (SIGPIPE, SIG_IGN);

	cerver_init ();

	cerver_version_print_full ();

	pocket_version_print_full ();

	if (!pocket_init ()) {
		start ();
	}

	else {
		cerver_log_error ("Failed to init pocket!");
	}

	(void) pocket_end ();

	cerver_end ();

	return 0;

}