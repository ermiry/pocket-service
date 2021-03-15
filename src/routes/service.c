#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cerver/types/types.h>

#include <cerver/http/http.h>
#include <cerver/http/request.h>
#include <cerver/http/response.h>

#include <cerver/utils/utils.h>
#include <cerver/utils/log.h>

#include "pocket.h"

#include "models/user.h"

// GET /api/pocket
void pocket_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	(void) http_response_send (pocket_works, http_receive);

}

// GET /api/pocket/version
void pocket_version_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	(void) http_response_send (current_version, http_receive);

}

// GET /api/pocket/auth
void pocket_auth_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;

	if (user) {
		#ifdef POCKET_DEBUG
		user_print (user);
		#endif

		(void) http_response_send (oki_doki, http_receive);
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
	}

}