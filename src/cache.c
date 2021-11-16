#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <credis/crud.h>
#include <credis/redis.h>

#include "pocket.h"
#include "runtime.h"
#include "version.h"

void pocket_cache_init (void) {

	// web:service_name:state
	(void) credis_command (
		"HMSET web:pocket:state "
		"runtime %s port %u "
		"cerver_receive_buffer_size %u cerver_th_threads %u "
		"cerver_connection_queue %u "
		"started %ld",
		runtime_to_string (RUNTIME), PORT,
		CERVER_RECEIVE_BUFFER_SIZE, CERVER_TH_THREADS,
		CERVER_CONNECTION_QUEUE,
		time (NULL)
	);

	// web:service_name:version
	(void) credis_command (
		"HMSET web:pocket:version "
		"name %s date %s time %s author %s",
		POCKET_VERSION_NAME, POCKET_VERSION_DATE,
		POCKET_VERSION_TIME, POCKET_VERSION_AUTHOR
	);

}

void pocket_cache_user_increment_transactions (
	const char *user_id
) {

	// pocket:user:user_id:stats
	credis_command (
		"HINCRBY pocket:user:%s:stats transactions 1",
		user_id
	);

}

void pocket_cache_user_increment_categories (
	const char *user_id
) {

	// pocket:user:user_id:stats
	credis_command (
		"HINCRBY pocket:user:%s:stats categories 1",
		user_id
	);

}

void pocket_cache_user_increment_places (
	const char *user_id
) {

	// pocket:user:user_id:stats
	credis_command (
		"HINCRBY pocket:user:%s:stats places 1",
		user_id
	);

}
