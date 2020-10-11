#include <stdio.h>

#include "version.h"

// print full pocket version information
void pocket_version_print_full (void) {

	printf ("\n\nTiny Pocket Version: %s\n", POCKET_VERSION_NAME);
	printf ("Release Date & time: %s - %s\n", POCKET_VERSION_DATE, POCKET_VERSION_TIME);
	printf ("Author: %s\n\n", POCKET_VERSION_AUTHOR);

}

// print the version id
void pocket_version_print_version_id (void) {

	printf ("\n\nTiny Pocket Version ID: %s\n", POCKET_VERSION);

}

// print the version name
void pocket_version_print_version_name (void) {

	printf ("\n\nTiny Pocket Version: %s\n", POCKET_VERSION_NAME);

}