#ifndef _POCKET_SERVICE_H_
#define _POCKET_SERVICE_H_

struct _HttpResponse;

extern struct _HttpResponse *missing_values;

extern struct _HttpResponse *pocket_works;
extern struct _HttpResponse *current_version;

extern struct _HttpResponse *catch_all;

extern unsigned int pocket_service_init (void);

extern void pocket_service_end (void);

#endif