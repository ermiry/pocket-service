#ifndef _POCKET_H_
#define _POCKET_H_

#include <stdbool.h>

#include "runtime.h"

#define MONGO_URI_SIZE					256
#define MONGO_APP_NAME_SIZE				32
#define MONGO_DB_SIZE					32

#define PUB_KEY_SIZE					128

#define REDIS_HOSTNAME_SIZE				128

extern RuntimeType RUNTIME;

extern unsigned int PORT;

extern unsigned int CERVER_RECEIVE_BUFFER_SIZE;
extern unsigned int CERVER_TH_THREADS;
extern unsigned int CERVER_CONNECTION_QUEUE;

extern const char *PUB_KEY;

extern bool CONNECT_TO_REDIS;
extern const char *REDIS_HOSTNAME;

// inits pocket main values
extern unsigned int pocket_init (void);

// ends pocket main values
extern unsigned int pocket_end (void);

#endif