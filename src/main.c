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
#include "things.h"
#include "version.h"

#include "models/user.h"

static Cerver *things_api = NULL;

void end (int dummy) {
	
	if (things_api) {
		cerver_stats_print (things_api, false, false);
		printf ("\nHTTP Cerver stats:\n");
		http_cerver_all_stats_print ((HttpCerver *) things_api->cerver_data);
		printf ("\n");
		cerver_teardown (things_api);
	}

	things_end ();

	exit (0);

}

static void things_set_things_routes (HttpCerver *http_cerver) {

	/* register top level route */
	// GET /api/things/
	HttpRoute *things_route = http_route_create (REQUEST_METHOD_GET, "api/things", things_handler);
	http_cerver_route_register (http_cerver, things_route);

	/* register things children routes */
	// GET api/things/version/
	HttpRoute *things_version_route = http_route_create (REQUEST_METHOD_GET, "version", things_version_handler);
	http_route_child_add (things_route, things_version_route);

	// GET api/things/auth/
	HttpRoute *things_auth_route = http_route_create (REQUEST_METHOD_GET, "auth", things_auth_handler);
	http_route_set_auth (things_auth_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (things_auth_route, user_parse_from_json, user_delete);
	http_route_child_add (things_route, things_auth_route);

}

static void start (void) {

	things_api = cerver_create (CERVER_TYPE_WEB, "things-api", atoi (PORT->str), PROTOCOL_TCP, false, 10, 1000);
	if (things_api) {
		/*** cerver configuration ***/
		cerver_set_receive_buffer_size (things_api, 4096);
		cerver_set_thpool_n_threads (things_api, 4);
		cerver_set_handler_type (things_api, CERVER_HANDLER_TYPE_THREADS);

		/*** web cerver configuration ***/
		HttpCerver *http_cerver = (HttpCerver *) things_api->cerver_data;

		http_cerver_auth_set_jwt_algorithm (http_cerver, JWT_ALG_RS256);
		http_cerver_auth_set_jwt_priv_key_filename (http_cerver, "keys/key.key");
		http_cerver_auth_set_jwt_pub_key_filename (http_cerver, "keys/key.pub");

		things_set_things_routes (http_cerver);

		// add a catch all route
		http_cerver_set_catch_all_route (http_cerver, things_catch_all_handler);

		if (cerver_start (things_api)) {
			char *s = c_string_create ("Failed to start %s!",
				things_api->info->name->str);
			if (s) {
				cerver_log_error (s);
				free (s);
			}

			cerver_delete (things_api);
		}
	}

	else {
		cerver_log_error ("Failed to create cerver!");

		cerver_delete (things_api);
	}

}

int main (int argc, char **argv) {

	srand (time (NULL));

	// register to the quit signal
	signal (SIGINT, end);
	signal (SIGTERM, end);

	cerver_version_print_full ();

	things_version_print_full ();

	if (!things_init ()) {
		start ();
	}

	else {
		cerver_log_error ("Failed to init things!");
	}

	return things_end ();

}