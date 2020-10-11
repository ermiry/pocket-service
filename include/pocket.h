#ifndef _POCKET_H_
#define _POCKET_H_

#include <cerver/types/types.h>
#include <cerver/types/string.h>

#include <cerver/handler.h>

#include <cerver/http/request.h>

#define DEFAULT_PORT					"5001"

#pragma region main

extern const String *PORT;

// inits pocket main values
extern unsigned int pocket_init (void);

// ends pocket main values
extern unsigned int pocket_end (void);

#pragma endregion

#pragma region routes

// GET api/pocket/
extern void pocket_handler (CerverReceive *cr, HttpRequest *request);

// GET api/pocket/version
extern void pocket_version_handler (CerverReceive *cr, HttpRequest *request);

// GET api/pocket/auth
extern void pocket_auth_handler (CerverReceive *cr, HttpRequest *request);

#pragma endregion

#endif