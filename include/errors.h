#ifndef _POCKET_ERRORS_H_
#define _POCKET_ERRORS_H_

#define POCKET_ERROR_MAP(XX)						\
	XX(0,	NONE, 				None)				\
	XX(1,	BAD_REQUEST, 		Bad Request)		\
	XX(2,	MISSING_VALUES, 	Missing Values)		\
	XX(3,	BAD_USER, 			Bad User)			\
	XX(4,	SERVER_ERROR, 		Server Error)

typedef enum PocketError {

	#define XX(num, name, string) POCKET_ERROR_##name = num,
	POCKET_ERROR_MAP (XX)
	#undef XX

} PocketError;

extern const char *pocket_error_to_string (PocketError type);

extern void pocket_error_send_response (
	PocketError error,
	const struct _HttpReceive *http_receive
);

#endif