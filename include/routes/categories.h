#ifndef _POCKET_ROUTES_CATEGORIES_H_
#define _POCKET_ROUTES_CATEGORIES_H_

struct _HttpReceive;
struct _HttpResponse;

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

// PUT /api/pocket/categories/:id
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

#endif