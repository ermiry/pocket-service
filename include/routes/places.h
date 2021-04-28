#ifndef _POCKET_ROUTES_PLACES_H_
#define _POCKET_ROUTES_PLACES_H_

struct _HttpReceive;
struct _HttpResponse;

// GET /api/pocket/places
// get all the authenticated user's places
extern void pocket_places_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// POST /api/pocket/places
// a user has requested to create a new place
extern void pocket_place_create_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// GET /api/pocket/places/:id/info
// returns information about an existing place that belongs to a user
extern void pocket_place_get_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// PUT /api/pocket/places/:id/update
// a user wants to update an existing place
extern void pocket_place_update_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// DELETE /api/pocket/places/:id/remove
// deletes an existing user's place
extern void pocket_place_delete_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

#endif