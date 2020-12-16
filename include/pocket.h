#ifndef _POCKET_H_
#define _POCKET_H_

#include <stdbool.h>

#include "runtime.h"

#define DEFAULT_PORT					"5001"

struct _HttpResponse;

extern RuntimeType RUNTIME;

extern unsigned int PORT;

extern unsigned int CERVER_RECEIVE_BUFFER_SIZE;
extern unsigned int CERVER_TH_THREADS;
extern unsigned int CERVER_CONNECTION_QUEUE;

extern const String *PRIV_KEY;
extern const String *PUB_KEY;

extern bool ENABLE_USERS_ROUTES;

extern struct _HttpResponse *oki_doki;
extern struct _HttpResponse *bad_request;
extern struct _HttpResponse *server_error;
extern struct _HttpResponse *bad_user;
extern struct _HttpResponse *missing_values;

extern struct _HttpResponse *pocket_works;
extern struct _HttpResponse *current_version;

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

extern struct _HttpResponse *no_user_places;
extern struct _HttpResponse *no_user_place;

extern struct _HttpResponse *place_created_success;
extern struct _HttpResponse *place_created_bad;
extern struct _HttpResponse *place_deleted_success;
extern struct _HttpResponse *place_deleted_bad;

// inits pocket main values
extern unsigned int pocket_init (void);

// ends pocket main values
extern unsigned int pocket_end (void);

#endif