#include <cerver/handler.h>

#include <cerver/http/http.h>
#include <cerver/http/response.h>

#include "pocket.h"
#include "errors.h"

#include "controllers/service.h"

const char *pocket_error_to_string (const PocketError type) {

	switch (type) {
		#define XX(num, name, string) case POCKET_ERROR_##name: return #string;
		POCKET_ERROR_MAP(XX)
		#undef XX
	}

	return pocket_error_to_string (POCKET_ERROR_NONE);

}

void pocket_error_send_response (
	const PocketError error,
	const HttpReceive *http_receive
) {

	switch (error) {
		case POCKET_ERROR_NONE: break;

		case POCKET_ERROR_BAD_REQUEST:
			(void) http_response_send (bad_request_error, http_receive);
			break;

		case POCKET_ERROR_MISSING_VALUES:
			(void) http_response_send (missing_values, http_receive);
			break;

		case POCKET_ERROR_BAD_USER:
			(void) http_response_send (bad_user_error, http_receive);
			break;

		case POCKET_ERROR_NOT_FOUND:
			(void) http_response_send (not_found_error, http_receive);
			break;

		case POCKET_ERROR_SERVER_ERROR:
			(void) http_response_send (server_error, http_receive);
			break;

		default: break;
	}

}