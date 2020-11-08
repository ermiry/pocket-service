#ifndef _POCKET_H_
#define _POCKET_H_

#include <cerver/types/types.h>
#include <cerver/types/string.h>

#include <cerver/handler.h>

#include <cerver/http/request.h>

#define DEFAULT_PORT					"5001"

#pragma region main

extern const String *PORT;

extern unsigned int CERVER_RECEIVE_BUFFER_SIZE;
extern unsigned int CERVER_TH_THREADS;

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

// GET api/pocket/transactions
// get all the authenticated user's transactions
extern void pocket_transactions_handler (CerverReceive *cr, HttpRequest *request);

// POST api/pocket/transactions
// a user has requested to create a new transaction
extern void pocket_transaction_create_handler (CerverReceive *cr, HttpRequest *request);

// GET api/pocket/transactions/:id
// returns information about an existing transaction that belongs to a user
extern void pocket_transaction_get_handler (CerverReceive *cr, HttpRequest *request);

// DELETE api/pocket/transactions/:id
// deletes an existing user's transaction
extern void pocket_transaction_delete_handler (CerverReceive *cr, HttpRequest *request);

#pragma endregion

#endif