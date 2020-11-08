#ifndef _POCKET_TRANSACTIONS_H_
#define _POCKET_TRANSACTIONS_H_

#include <cerver/collections/pool.h>

#include "models/transaction.h"

#define DEFAULT_TRANS_POOL_INIT			32

extern Pool *trans_pool;

extern unsigned int pocket_trans_init (void);

extern void pocket_trans_end (void);

extern Transaction *pocket_trans_create (
	const char *title,
	const double amount
);

extern void pocket_trans_delete (void *trans_ptr);

#endif