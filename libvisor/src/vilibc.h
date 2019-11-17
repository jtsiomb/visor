#ifndef VISOR_LIBC_H_
#define VISOR_LIBC_H_

/* XXX let's pretend we don't have a libc to test our own code
#ifdef __STDC_HOSTED__
#define HAVE_LIBC
#endif
*/

#ifdef HAVE_LIBC
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#else

void *memset(void *s, int c, unsigned long n);
void *memcpy(void *dest, const void *src, unsigned long n);
void *memmove(void *dest, const void *src, unsigned long n);
unsigned long strlen(const char *s);
int strcmp(const char *s1, const char *s2);

#ifdef __GNUC__
typedef __builtin_va_list va_list;
#define va_start(v,l)	__builtin_va_start(v,l)
#define va_end(v)	__builtin_va_end(v)
#define va_arg(v,l)	__builtin_va_arg(v,l)
#else	/* !def __GNUC__ */
#error "stdargs implementation for this compiler missing (libvisor/src/vilibc.h)"
#endif

int sprintf(char *buf, const char *fmt, ...);
int vsprintf(char *buf, const char *fmt, va_list ap);
int snprintf(char *buf, unsigned long sz, const char *fmt, ...);
int vsnprintf(char *buf, unsigned long sz, const char *fmt, va_list ap);

#endif	/* !HAVE_LIBC */

struct visor;
void vi_error(struct visor *vi, const char *fmt, ...);

#endif	/* VISOR_LIBC_H_ */