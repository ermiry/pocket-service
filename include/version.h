#ifndef _THINGS_VERSION_H_
#define _THINGS_VERSION_H_

#define THINGS_VERSION                    	"0.1"
#define THINGS_VERSION_NAME               	"Version 0.1"
#define THINGS_VERSION_DATE			    	"10/10/2020"
#define THINGS_VERSION_TIME			    	"22:32 CST"
#define THINGS_VERSION_AUTHOR			    "Erick Salas"

// print full things version information
extern void things_version_print_full (void);

// print the version id
extern void things_version_print_version_id (void);

// print the version name
extern void things_version_print_version_name (void);

#endif