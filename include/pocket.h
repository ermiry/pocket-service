#ifndef _POCKET_H_
#define _POCKET_H_

#include <cerver/types/types.h>
#include <cerver/types/string.h>

#define DEFAULT_PORT					"5001"

struct _HttpReceive;
struct _HttpRequest;

extern unsigned int PORT;

extern unsigned int CERVER_RECEIVE_BUFFER_SIZE;
extern unsigned int CERVER_TH_THREADS;
extern unsigned int CERVER_CONNECTION_QUEUE;

extern bool ENABLE_USERS_ROUTES;

extern HttpResponse *oki_doki;
extern HttpResponse *bad_request;
extern HttpResponse *server_error;
extern HttpResponse *bad_user;
extern HttpResponse *missing_values;

#pragma region main

// inits pocket main values
extern unsigned int pocket_init (void);

// ends pocket main values
extern unsigned int pocket_end (void);

#pragma endregion

#pragma region routes

// GET /api/pocket
extern void pocket_handler (
    const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// GET /api/pocket/version
extern void pocket_version_handler (
    const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// GET /api/pocket/auth
extern void pocket_auth_handler (
    const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// GET /api/pocket/transactions
// get all the authenticated user's transactions
extern void pocket_transactions_handler (
    const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// POST /api/pocket/transactions
// a user has requested to create a new transaction
extern void pocket_transaction_create_handler (
    const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// GET /api/pocket/transactions/:id
// returns information about an existing transaction that belongs to a user
extern void pocket_transaction_get_handler (
    const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// POST /api/pocket/transactions/:id
// a user wants to update an existing transaction
extern void pocket_transaction_update_handler (
    const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// DELETE /api/pocket/transactions/:id
// deletes an existing user's transaction
extern void pocket_transaction_delete_handler (
    const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// GET /api/pocket/categories
// get all the authenticated user's categories
extern void pocket_categories_handler (
    const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// POST /api/pocket/categories
// a user has requested to create a new category
extern void pocket_category_create_handler (
    const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// GET /api/pocket/categories/:id
// returns information about an existing category that belongs to a user
extern void pocket_category_get_handler (
    const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// POST /api/pocket/categories/:id
// a user wants to update an existing category
extern void pocket_category_update_handler (
    const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// DELETE /api/pocket/categories/:id
// deletes an existing user's category
extern void pocket_category_delete_handler (
    const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

#pragma endregion

#endif