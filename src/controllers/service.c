#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cerver/http/response.h>

#include "version.h"

#include "controllers/service.h"

HttpResponse *missing_values = NULL;

HttpResponse *pocket_works = NULL;
HttpResponse *current_version = NULL;

static char version_buffer[VERSION_RESPONSE_SIZE] = { 0 };

unsigned int pocket_service_init (void) {

	unsigned int retval = 1;

	missing_values = http_response_json_key_value (
		HTTP_STATUS_BAD_REQUEST, "error", "Missing values!"
	);

	pocket_works = http_response_json_key_value (
		HTTP_STATUS_OK, "msg", "Pocket works!"
	);

	(void) snprintf (
		version_buffer, VERSION_RESPONSE_SIZE,
		"%s - %s", POCKET_VERSION_NAME, POCKET_VERSION_DATE
	);

	current_version = http_response_json_key_value (
		HTTP_STATUS_OK, "version", version_buffer
	);

	if (
		missing_values
		&& pocket_works && current_version
	) retval = 0;

	return retval;

}

void pocket_service_end (void) {

	http_response_delete (missing_values);

	http_response_delete (pocket_works);
	http_response_delete (current_version);

}
