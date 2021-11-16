#ifndef _POCKET_SERVICE_H_
#define _POCKET_SERVICE_H_

#define VERSION_RESPONSE_SIZE		64

struct _HttpResponse;

extern struct _HttpResponse *missing_values;

extern struct _HttpResponse *pocket_works;
extern struct _HttpResponse *current_version;

extern unsigned int pocket_service_init (void);

extern void pocket_service_end (void);

#endif