/*
visor - lightweight system-independent embeddable text editor framework
Copyright (C)  2019 John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
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
#include <ctype.h>
#include <limit.h>
#include <assert.h>
#else

int atoi(const char *str);
long atol(const char *str);
long strtol(const char *str, char **endp, int base);

void *memset(void *s, int c, unsigned long n);
void *memcpy(void *dest, const void *src, unsigned long n);
void *memmove(void *dest, const void *src, unsigned long n);
unsigned long strlen(const char *s);
char *strchr(const char *s, int c);
int strcmp(const char *s1, const char *s2);
char *strcpy(char *dest, const char *src);

#ifdef __GNUC__
typedef __builtin_va_list va_list;
#define va_start(v,l)	__builtin_va_start(v,l)
#define va_end(v)		__builtin_va_end(v)
#define va_arg(v,l)		__builtin_va_arg(v,l)
#else	/* !def __GNUC__ */
#error "stdargs implementation for this compiler missing (libvisor/src/vilibc.h)"
#endif

int sprintf(char *buf, const char *fmt, ...);
int vsprintf(char *buf, const char *fmt, va_list ap);
int snprintf(char *buf, unsigned long sz, const char *fmt, ...);
int vsnprintf(char *buf, unsigned long sz, const char *fmt, va_list ap);

int isalnum(int c);
int isalpha(int c);
#define isascii(c)	((c) < 128)
int isblank(int c);
int isdigit(int c);
int isupper(int c);
int islower(int c);
int isprint(int c);
int isspace(int c);

int toupper(int c);
int tolower(int c);

#define assert(x)

#endif	/* !HAVE_LIBC */

struct visor;
void vi_error(struct visor *vi, const char *fmt, ...);

#endif	/* VISOR_LIBC_H_ */
