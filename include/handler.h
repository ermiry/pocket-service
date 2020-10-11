#ifndef _POCKET_HANDLER_H_
#define _POCKET_HANDLER_H_

#include <cerver/handler.h>

#include <cerver/http/request.h>

// GET *
extern void pocket_catch_all_handler (CerverReceive *cr, HttpRequest *request);

#endif