#ifndef _THINGS_H_
#define _THINGS_H_

#include "cerver/types/types.h"
#include "cerver/types/string.h"

#define DEFAULT_PORT					"5001"

#pragma region main

extern const String *PORT;

// inits things main values
extern unsigned int things_init (void);

// ends things main values
extern unsigned int things_end (void);

#pragma endregion

#endif