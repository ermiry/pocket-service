#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <credis/crud.h>
#include <credis/redis.h>

#include "pocket.h"
#include "version.h"

void pocket_cache_init (void) {

	// web:service_name:state
	(void) credis_command (
		"HMSET web:pocket:state "
		"runtime %d port %u start_time %ld "
		"cerver_receive_buffer_size %u cerver_th_threads %u "
		"cerver_connection_queue %u "
		"started %ld",
		RUNTIME, PORT, time (NULL),
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
