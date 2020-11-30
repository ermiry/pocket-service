#ifndef _POCKET_H_
#define _POCKET_H_

#include <cerver/types/types.h>
#include <cerver/types/string.h>

#define DEFAULT_PORT					"5001"

struct _HttpReceive;
struct _HttpRequest;
struct _HttpResponse;

extern unsigned int PORT;

extern unsigned int CERVER_RECEIVE_BUFFER_SIZE;
extern unsigned int CERVER_TH_THREADS;
extern unsigned int CERVER_CONNECTION_QUEUE;

extern bool ENABLE_USERS_ROUTES;

extern struct _HttpResponse *oki_doki;
extern struct _HttpResponse *bad_request;
extern struct _HttpResponse *server_error;
extern struct _HttpResponse *bad_user;
extern struct _HttpResponse *missing_values;

extern struct _HttpResponse *no_user_trans;

extern struct _HttpResponse *trans_created_success;
extern struct _HttpResponse *trans_created_bad;
extern struct _HttpResponse *trans_deleted_success;
extern struct _HttpResponse *trans_deleted_bad;

extern struct _HttpResponse *no_user_categories;
extern struct _HttpResponse *no_user_category;

extern struct _HttpResponse *category_created_success;
extern struct _HttpResponse *category_created_bad;
extern struct _HttpResponse *category_deleted_success;
extern struct _HttpResponse *category_deleted_bad;

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

#pragma endregion

#endif