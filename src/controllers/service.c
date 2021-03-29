#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cerver/http/response.h>

#include <cerver/utils/utils.h>

#include "version.h"

HttpResponse *missing_values = NULL;

HttpResponse *pocket_works = NULL;
HttpResponse *current_version = NULL;

unsigned int pocket_service_init (void) {

	unsigned int retval = 1;

	missing_values = http_response_json_key_value (
		(http_status) 400, "error", "Missing values!"
	);

	pocket_works = http_response_json_key_value (
		(http_status) 200, "msg", "Pocket works!"
	);

	char *status = c_string_create (
		"%s - %s", POCKET_VERSION_NAME, POCKET_VERSION_DATE
	);

	if (status) {
		current_version = http_response_json_key_value (
			(http_status) 200, "version", status
		);

		free (status);
	}

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
