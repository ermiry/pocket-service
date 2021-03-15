#include <cerver/http/http.h>
#include <cerver/http/request.h>
#include <cerver/http/response.h>

static HttpResponse *catch_all = NULL;

unsigned int pocket_handler_init (void) {

	unsigned int retval = 1;

	catch_all = http_response_json_key_value (
		(http_status) 200, "msg", "Tiny Pocket Service!"
	);

	if (catch_all) retval = 0;

	return retval;

}

void pocket_handler_end (void) {

	http_response_delete (catch_all);

}

// GET *
void pocket_catch_all_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	http_response_send (catch_all, http_receive);

}