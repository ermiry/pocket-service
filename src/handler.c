#include <cerver/handler.h>

#include <cerver/http/request.h>
#include <cerver/http/response.h>

// GET *
void things_catch_all_handler (CerverReceive *cr, HttpRequest *request) {

	HttpResponse *res = http_response_json_msg (200, "Tiny Things Service!");
	if (res) {
		// http_response_print (res);
		http_response_send (res, cr->cerver, cr->connection);
		http_respponse_delete (res);
	}

}