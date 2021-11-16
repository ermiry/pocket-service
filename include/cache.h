#ifndef _POCKET_CACHE_H_
#define _POCKET_CACHE_H_

extern void pocket_cache_init (void);

extern void pocket_cache_user_increment_transactions (
	const char *user_id
);

extern void pocket_cache_user_increment_categories (
	const char *user_id
);

extern void pocket_cache_user_increment_places (
	const char *user_id
);

#endif