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
#include "vilibc.h"
#include "vimpl.h"

#ifndef HAVE_LIBC

int atoi(const char *str)
{
	return strtol(str, 0, 10);
}

long atol(const char *str)
{
	return strtol(str, 0, 10);
}

long strtol(const char *str, char **endp, int base)
{
	long acc = 0;
	int sign = 1;
	int valid = 0;
	const char *start = str;

	while(isspace(*str)) str++;

	if(base == 0) {
		if(str[0] == '0') {
			if(str[1] == 'x' || str[1] == 'X') {
				base = 16;
			} else {
				base = 8;
			}
		} else {
			base = 10;
		}
	}

	if(*str == '+') {
		str++;
	} else if(*str == '-') {
		sign = -1;
		str++;
	}

	while(*str) {
		long val = 0x7fffffff;
		char c = tolower(*str);

		if(isdigit(c)) {
			val = *str - '0';
		} else if(c >= 'a' && c <= 'f') {
			val = 10 + c - 'a';
		} else {
			break;
		}
		if(val >= base) {
			break;
		}
		valid = 1;

		acc = acc * base + val;
		str++;
	}

	if(endp) {
		*endp = (char*)(valid ? str : start);
	}

	return sign > 0 ? acc : -acc;
}


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

char *strchr(const char *s, int c)
{
	while(*s) {
		if(*s == c) {
			return (char*)s;
		}
		s++;
	}
	return 0;
}

int strcmp(const char *s1, const char *s2)
{
	while(*s1 && *s1 == *s2) {
		s1++;
		s2++;
	}
	return *s1 - *s2;
}

char *strcpy(char *dest, const char *src)
{
	char *dptr = dest;
	while((*dptr++ = *src++));
	return dest;
}


int isalnum(int c)
{
	return isalpha(c) || isdigit(c);
}

int isalpha(int c)
{
	return isupper(c) || islower(c);
}

int isblank(int c)
{
	return c == ' ' || c == '\t';
}

int isdigit(int c)
{
	return c >= '0' && c <= '9';
}

int isupper(int c)
{
	return c >= 'A' && c <= 'Z';
}

int islower(int c)
{
	return c >= 'a' && c <= 'z';
}

int isgraph(int c)
{
	return c > ' ' && c <= '~';
}

int isprint(int c)
{
	return isgraph(c) || c == ' ';
}

int isspace(int c)
{
	return isblank(c) || c == '\f' || c == '\n' || c == '\r' || c == '\v';
}

int toupper(int c)
{
	return islower(c) ? (c + ('A' - 'a')) : c;
}

int tolower(int c)
{
	return isupper(c) ? (c - ('A' - 'a')) : c;
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
