#include <cerver/utils/log.h>

#include "version.h"

// print full pocket version information
void pocket_version_print_full (void) {

	cerver_log_both (
		LOG_TYPE_NONE, LOG_TYPE_NONE,
		"Tiny Pocket Version: %s", POCKET_VERSION_NAME
	);

	cerver_log_both (
		LOG_TYPE_NONE, LOG_TYPE_NONE,
		"Release Date & time: %s - %s",
		POCKET_VERSION_DATE, POCKET_VERSION_TIME
	);

	cerver_log_both (
		LOG_TYPE_NONE, LOG_TYPE_NONE,
		"Author: %s\n", POCKET_VERSION_AUTHOR
	);

}

// print the version id
void pocket_version_print_version_id (void) {

	cerver_log_both (
		LOG_TYPE_NONE, LOG_TYPE_NONE,
		"Tiny Pocket Version ID: %s\n", POCKET_VERSION
	);

}

// print the version name
void pocket_version_print_version_name (void) {

	cerver_log_both (
		LOG_TYPE_NONE, LOG_TYPE_NONE,
		"Tiny Pocket Version: %s\n", POCKET_VERSION_NAME
	);

}
