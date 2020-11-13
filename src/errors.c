#include <cerver/handler.h>

#include <cerver/http/response.h>

#include "pocket.h"
#include "errors.h"

const char *pocket_error_to_string (PocketError type) {

	switch (type) {
		#define XX(num, name, string) case POCKET_ERROR_##name: return #string;
		POCKET_ERROR_MAP(XX)
		#undef XX
	}

	return pocket_error_to_string (POCKET_ERROR_NONE);

}

void pocket_error_send_response (PocketError error, CerverReceive *cr) {

	switch (error) {
		case POCKET_ERROR_NONE: break;

		case POCKET_ERROR_BAD_REQUEST:
			(void) http_response_send (bad_request, cr->cerver, cr->connection);
			break;

		case POCKET_ERROR_MISSING_VALUES:
			(void) http_response_send (missing_values, cr->cerver, cr->connection);
			break;

		case POCKET_ERROR_BAD_USER:
			(void) http_response_send (bad_user, cr->cerver, cr->connection);
			break;

		case POCKET_ERROR_SERVER_ERROR:
			(void) http_response_send (server_error, cr->cerver, cr->connection);
			break;

		default: break;
	}

}