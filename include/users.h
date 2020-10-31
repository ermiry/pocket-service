#ifndef _POCKET_USERS_H_
#define _POCKET_USERS_H_

#include <cerver/collections/dlist.h>

#include <cerver/handler.h>

#include <cerver/http/request.h>

#pragma region main

extern unsigned int pocket_users_init (void);

extern void pocket_users_end (void);

#pragma endregion

#pragma region routes

// GET api/users/
extern void users_handler (CerverReceive *cr, HttpRequest *request);

// POST api/users/login
extern void users_login_handler (CerverReceive *cr, HttpRequest *request);

// POST api/users/register
extern void users_register_handler (CerverReceive *cr, HttpRequest *request);

#pragma endregion

#endif