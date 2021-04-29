#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <cerver/http/json/json.h>

#include "curl.h"
#include "test.h"

#define ADDRESS_SIZE		128

static const char *address = { "127.0.0.1:5000/api/users" };

static const char *name = { "Erick Salas" };
static const char *username = { "ermiry" };
static const char *email = { "test@ermiry.com" };
static const char *password = { "049ec1af7c1332193d602986f2fdad5b4d1c2ff90e5cdc65388c794c1f10226b" };

static size_t users_request_data_handler (
	void *contents, size_t size, size_t nmemb, void *storage
) {

	(void) strncpy ((char *) storage, (char *) contents, size * nmemb);

	return size * nmemb;

}

static char *users_register_create_complete (void) {

	char *json = NULL;

	json_t *register_values = json_pack (
		"{s:s, s:s, s:s, s:s, s:s}",
		"name", name,
		"username", username,
		"email", email,
		"password", password,
		"confirm", password
	);

	if (register_values) {
		json = json_dumps (register_values, 0);

		json_decref (register_values);
	}

	return json;

}

// POST api/users/register
static unsigned int users_request_register_complete (
	CURL *curl, const char *actual_address
) {

	unsigned int retval = 1;

	char *json = users_register_create_complete ();
	if (json) {
		retval = curl_simple_post_json (
			curl, actual_address,
			json, strlen (json)
		);
		
		free (json);
	}

	return retval;

}

static char *users_login_create_complete (void) {

	char *json = NULL;

	json_t *register_values = json_pack (
		"{s:s, s:s}",
		"email", email,
		"password", password
	);

	if (register_values) {
		json = json_dumps (register_values, 0);

		json_decref (register_values);
	}

	return json;

}

// POST api/users/login
static unsigned int users_request_login_complete (
	CURL *curl, const char *actual_address
) {

	unsigned int retval = 1;

	char *json = users_login_create_complete ();
	if (json) {
		retval = curl_simple_post_json (
			curl, actual_address,
			json, strlen (json)
		);
		
		free (json);
	}

	return retval;

}

static void users_request_perform (void) {

	char data_buffer[4096] = { 0 };
	char actual_address[ADDRESS_SIZE] = { 0 };

	CURL *curl = curl_easy_init ();

	// POST api/users/register
	(void) snprintf (actual_address, ADDRESS_SIZE - 1, "%s/register", address);
	test_check_unsigned_eq (users_request_register_complete (curl, actual_address), 0, NULL);

	// POST api/users/login
	(void) snprintf (actual_address, ADDRESS_SIZE - 1, "%s/login", address);
	test_check_unsigned_eq (users_request_login_complete (curl, actual_address), 0, NULL);

	curl_easy_cleanup (curl);

}

int main (int argc, char **argv) {

	(void) printf ("Requesting users...\n");

	users_request_perform ();

	(void) printf ("Done!\n");

	return 0;

}
