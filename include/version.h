#ifndef _POCKET_VERSION_H_
#define _POCKET_VERSION_H_

#define POCKET_VERSION                    	"0.4"
#define POCKET_VERSION_NAME               	"Version 0.4"
#define POCKET_VERSION_DATE			    	"16/12/2020"
#define POCKET_VERSION_TIME			    	"11:51 CST"
#define POCKET_VERSION_AUTHOR			    "Erick Salas"

// print full pocket version information
extern void pocket_version_print_full (void);

// print the version id
extern void pocket_version_print_version_id (void);

// print the version name
extern void pocket_version_print_version_name (void);

#endif