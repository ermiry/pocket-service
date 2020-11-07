#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <cerver/types/types.h>

#include <cerver/utils/log.h>

#include "mongo.h"

#include "models/transaction.h"

#define transactionS_COLL_NAME         				"transactions"

mongoc_collection_t *transactions_collection = NULL;

// opens handle to transaction collection
unsigned int transactions_collection_get (void) {

	unsigned int retval = 1;

	transactions_collection = mongo_collection_get (transactionS_COLL_NAME);
	if (transactions_collection) {
		retval = 0;
	}

	else {
		cerver_log_error ("Failed to get handle to transactions collection!");
	}

	return retval;

}

void transactions_collection_close (void) {

	if (transactions_collection) mongoc_collection_destroy (transactions_collection);

}

void *transaction_new (void) {

	Transaction *transaction = (Transaction *) malloc (sizeof (Transaction));
	if (transaction) {
		memset (transaction, 0, sizeof (Transaction));
	}

	return transaction;

}

void transaction_delete (void *transaction_ptr) {

	if (transaction_ptr) free (transaction_ptr);

}

void transaction_print (Transaction *transaction) {

	if (transaction) {
		char buffer[128] = { 0 };
		bson_oid_to_string (&transaction->oid, buffer);
		printf ("id: %s\n", buffer);

		printf ("title: %s\n", transaction->title);
		printf ("amount: %.4f\n", transaction->amount);

		strftime (buffer, 128, "%d/%m/%y - %T", gmtime (&transaction->date));
		printf ("date: %s GMT\n", buffer);
	}

}