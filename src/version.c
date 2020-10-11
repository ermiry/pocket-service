#include <stdio.h>

#include "version.h"

// print full things version information
void things_version_print_full (void) {

	printf ("\n\nTiny Things Version: %s\n", THINGS_VERSION_NAME);
	printf ("Release Date & time: %s - %s\n", THINGS_VERSION_DATE, THINGS_VERSION_TIME);
	printf ("Author: %s\n\n", THINGS_VERSION_AUTHOR);

}

// print the version id
void things_version_print_version_id (void) {

	printf ("\n\nTiny Things Version ID: %s\n", THINGS_VERSION);

}

// print the version name
void things_version_print_version_name (void) {

	printf ("\n\nTiny Things Version: %s\n", THINGS_VERSION_NAME);

}