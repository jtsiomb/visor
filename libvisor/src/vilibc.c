#include "vilibc.h"
#include "vimpl.h"

#ifndef HAVE_LIBC

void *memset(void *s, int c, unsigned long n)
{
	char *p = s;
	while(n--) *p++ = c;
	return s;
}

void *memcpy(void *dest, const void *src, unsigned long n)
{
	char *d = dest;
	const char *s = src;
	while(n--) *d++ = *s++;
	return dest;
}

void *memmove(void *dest, const void *src, unsigned long n)
{
	unsigned long i;
	char *dptr;
	const char *sptr;

	if(dest <= src) {
		/* forward copy */
		dptr = dest;
		sptr = src;
		for(i=0; i<n; i++) {
			*dptr++ = *sptr++;
		}
	} else {
		/* backwards copy */
		dptr = (char*)dest + n - 1;
		sptr = (const char*)src + n - 1;
		for(i=0; i<n; i++) {
			*dptr-- = *sptr--;
		}
	}

	return dest;
}

unsigned long strlen(const char *s)
{
	unsigned long len = 0;
	while(*s++) len++;
	return len;
}

int strcmp(const char *s1, const char *s2)
{
	while(*s1 && *s1 == *s2) {
		s1++;
		s2++;
	}
	return *s1 - *s2;
}

#endif	/* !def HAVE_LIBC */

static char errstr_buf[256];

void vi_error(struct visor *vi, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(errstr_buf, sizeof errstr_buf, fmt, ap);
	va_end(ap);

	if(vi->tty.status) {
		vi->tty.status(errstr_buf, vi->tty_cls);
	}
}
