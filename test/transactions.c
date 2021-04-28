#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "curl.h"
#include "pocket.h"
#include "test.h"

#define ADDRESS_SIZE		128

static const char *address = { "127.0.0.1:5000/api/pocket/transactions" };

static size_t transactions_request_data_handler (
	void *contents, size_t size, size_t nmemb, void *storage
) {

	(void) strncpy ((char *) storage, (char *) contents, size * nmemb);

	return size * nmemb;

}

// GET api/pocket/transactions
static unsigned int transactions_request_all (
	CURL *curl, const char *actual_address
) {

	return curl_simple_with_auth (
		curl, actual_address,
		token
	);

}

static void transactions_request_perform (void) {

	char data_buffer[4096] = { 0 };
	char actual_address[ADDRESS_SIZE] = { 0 };

	CURL *curl = curl_easy_init ();

	// GET api/pocket/transactions
	(void) snprintf (actual_address, ADDRESS_SIZE - 1, "%s", address);
	test_check_unsigned_eq (transactions_request_all (curl, actual_address), 0, NULL);

	curl_easy_cleanup (curl);

}

int main (int argc, char **argv) {

	(void) printf ("Requesting transactions...\n");

	transactions_request_perform ();

	(void) printf ("Done!\n");

	return 0;

}
