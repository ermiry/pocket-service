#ifndef _POCKET_H_
#define _POCKET_H_

#include <stdbool.h>

#include "runtime.h"

#define DEFAULT_PORT					"5001"

struct _HttpCerver;
struct _HttpResponse;

extern struct _HttpCerver *http_cerver;

extern RuntimeType RUNTIME;

extern unsigned int PORT;

extern unsigned int CERVER_RECEIVE_BUFFER_SIZE;
extern unsigned int CERVER_TH_THREADS;
extern unsigned int CERVER_CONNECTION_QUEUE;

extern const String *PRIV_KEY;
extern const String *PUB_KEY;

extern bool ENABLE_USERS_ROUTES;

// inits pocket main values
extern unsigned int pocket_init (void);

// ends pocket main values
extern unsigned int pocket_end (void);

#endif