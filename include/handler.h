#ifndef _POCKET_HANDLER_H_
#define _POCKET_HANDLER_H_

struct _HttpReceive;
struct _HttpRequest;

extern unsigned int pocket_handler_init (void);

extern void pocket_handler_end (void);

// GET *
extern void pocket_catch_all_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

#endif