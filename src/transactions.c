#include <stdlib.h>

#include <cerver/collections/pool.h>

#include <cerver/http/json/json.h>

#include <cerver/utils/log.h>

#include "transactions.h"

#include "models/transaction.h"

Pool *trans_pool = NULL;

unsigned int pocket_trans_init (void) {

	unsigned int retval = 1;

	trans_pool = pool_create (transaction_delete);
	if (trans_pool) {
		pool_set_create (trans_pool, transaction_new);
		pool_set_produce_if_empty (trans_pool, true);
		if (!pool_init (trans_pool, transaction_new, DEFAULT_TRANS_POOL_INIT)) {
			retval = 0;
		}

		else {
			cerver_log_error ("Failed to init trans pool!");
		}
	}

	else {
		cerver_log_error ("Failed to create trans pool!");
	}

	return retval;

}

void pocket_trans_end (void) {

	pool_delete (trans_pool);
	trans_pool = NULL;

}

void pocket_trans_delete (void *trans_ptr) {

	memset (trans_ptr, 0, sizeof (Transaction));
	pool_push (trans_pool, trans_ptr);

}