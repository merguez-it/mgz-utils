#ifndef __MGZ_INTERNALSTRPTIME_H
#define __MGZ_INTERNALSTRPTIME_H

#include "config.h"

#ifndef HAVE_STRPTIME
#include <time.h>
#ifdef __cplusplus
extern "C" {
#endif
char * strptime (const char *buf, const char *format, struct tm *tm);
#ifdef __cplusplus
}
#endif
#endif // HAVE_STRPTIME

#endif // __MGZ_INTERNALSTRPTIME_H
