#ifndef _POCKET_ROUTES_TRANSACTIONS_H_
#define _POCKET_ROUTES_TRANSACTIONS_H_

struct _HttpReceive;
struct _HttpResponse;

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

// GET /api/pocket/transactions/:id/info
// returns information about an existing transaction that belongs to a user
extern void pocket_transaction_get_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// PUT /api/pocket/transactions/:id/update
// a user wants to update an existing transaction
extern void pocket_transaction_update_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// DELETE /api/pocket/transactions/:id/remove
// deletes an existing user's transaction
extern void pocket_transaction_delete_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

#endif