#ifndef _THINGS_HANDLER_H_
#define _THINGS_HANDLER_H_

#include <cerver/handler.h>

#include <cerver/http/request.h>

// GET *
extern void things_catch_all_handler (CerverReceive *cr, HttpRequest *request);

#endif