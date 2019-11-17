#include "vilibc.h"

enum {
	OUT_BUF
};

static int intern_printf(int out, char *buf, unsigned long sz, const char *fmt, va_list ap);
static void bwrite(int out, char *buf, unsigned long buf_sz, char *str, int sz);
static void utoa(unsigned int val, char *buf, int base);
static void itoa(int val, char *buf, int base);

int sprintf(char *buf, const char *fmt, ...)
{
	int res;
	va_list ap;

	va_start(ap, fmt);
	res = intern_printf(OUT_BUF, buf, 0, fmt, ap);
	va_end(ap);
	return res;
}

int vsprintf(char *buf, const char *fmt, va_list ap)
{
	return intern_printf(OUT_BUF, buf, 0, fmt, ap);
}

int snprintf(char *buf, unsigned long sz, const char *fmt, ...)
{
	int res;
	va_list ap;

	va_start(ap, fmt);
	res = intern_printf(OUT_BUF, buf, sz, fmt, ap);
	va_end(ap);
	return res;
}

int vsnprintf(char *buf, unsigned long sz, const char *fmt, va_list ap)
{
	return intern_printf(OUT_BUF, buf, sz, fmt, ap);
}

/* intern_printf provides all the functionality needed by all the printf
 * variants.
 * - buf: optional buffer onto which the formatted results are written. If null
 *   then the output goes to the terminal through putchar calls. This is used
 *   by the (v)sprintf variants which write to an array of char.
 * - sz: optional maximum size of the output, 0 means unlimited. This is used
 *   by the (v)snprintf variants to avoid buffer overflows.
 * The rest are obvious, format string and variable argument list.
 */
static char *convc = "dioxXucsfeEgGpn%";

#define IS_CONV(c)	strchr(convc, c)

#define BUF(x)	((x) ? (x) + cnum : (x))
#define SZ(x)	((x) ? (x) - cnum : (x))

static int intern_printf(int out, char *buf, unsigned long sz, const char *fmt, va_list ap)
{
	char conv_buf[32];
	char *str;
	int i, slen;
	const char *fstart = 0;

	/* state */
	int cnum = 0;
	int base = 10;
	int alt = 0;
	int fwidth = 0;
	int padc = ' ';
	int sign = 0;
	int left_align = 0;
	int hex_caps = 0;
	int unsig = 0;
	int num, unum;

	while(*fmt) {
		if(*fmt == '%') {
			fstart = fmt++;
			continue;
		}

		if(fstart) {
			if(IS_CONV(*fmt)) {
				switch(*fmt) {
				case 'X':
					hex_caps = 1;

				case 'x':
				case 'p':
					base = 16;

					if(alt) {
						bwrite(out, BUF(buf), SZ(sz), "0x", 2);
						cnum += 2;
					}

				case 'u':
					unsig = 1;

					if(0) {
				case 'o':
						base = 8;

						if(alt) {
							bwrite(out, BUF(buf), SZ(sz), "0", 1);
							cnum++;
						}
					}

				case 'd':
				case 'i':
					if(unsig) {
						unum = va_arg(ap, unsigned int);
						utoa(unum, conv_buf, base);
					} else {
						num = va_arg(ap, int);
						itoa(num, conv_buf, base);
					}
					if(hex_caps) {
						for(i=0; conv_buf[i]; i++) {
							conv_buf[i] = toupper(conv_buf[i]);
						}
					}

					slen = strlen(conv_buf);

					if(left_align) {
						if(!unsig && sign && num >= 0) {
							bwrite(out, BUF(buf), SZ(sz), "+", 1);
							cnum++;
						}
						bwrite(out, BUF(buf), SZ(sz), conv_buf, slen);
						cnum += slen;
						padc = ' ';
					}
					for(i=slen; i<fwidth; i++) {
						bwrite(out, BUF(buf), SZ(sz), (char*)&padc, 1);
						cnum++;
					}
					if(!left_align) {
						if(!unsig && sign && num >= 0) {
							bwrite(out, BUF(buf), SZ(sz), "+", 1);
							cnum++;
						}
						bwrite(out, BUF(buf), SZ(sz), conv_buf, slen);
						cnum += slen;
					}
					break;

				case 'c':
					{
						char c = va_arg(ap, int);
						bwrite(out, BUF(buf), SZ(sz), &c, 1);
						cnum++;
					}
					break;

				case 's':
					str = va_arg(ap, char*);
					slen = strlen(str);

					if(left_align) {
						bwrite(out, BUF(buf), SZ(sz), str, slen);
						cnum += slen;
						padc = ' ';
					}
					for(i=slen; i<fwidth; i++) {
						bwrite(out, BUF(buf), SZ(sz), (char*)&padc, 1);
						cnum++;
					}
					if(!left_align) {
						bwrite(out, BUF(buf), SZ(sz), str, slen);
						cnum += slen;
					}
					break;

				case 'n':
					*va_arg(ap, int*) = cnum;
					break;

				default:
					break;
				}

				/* restore default conversion state */
				base = 10;
				alt = 0;
				fwidth = 0;
				padc = ' ';
				hex_caps = 0;

				fstart = 0;
				fmt++;
			} else {
				switch(*fmt) {
				case '#':
					alt = 1;
					break;

				case '+':
					sign = 1;
					break;

				case '-':
					left_align = 1;
					break;

				case 'l':
				case 'L':
					break;

				case '0':
					padc = '0';
					break;

				default:
					if(isdigit(*fmt)) {
						const char *fw = fmt;
						while(*fmt && isdigit(*fmt)) fmt++;

						fwidth = atoi(fw);
						continue;
					}
				}
				fmt++;
			}
		} else {
			bwrite(out, BUF(buf), SZ(sz), (char*)fmt++, 1);
			cnum++;
		}
	}

	return cnum;
}

/* bwrite is called by intern_printf to transparently handle writing into a
 * buffer or to the terminal
 */
static void bwrite(int out, char *buf, unsigned long buf_sz, char *str, int sz)
{
	if(out == OUT_BUF) {
		if(buf_sz && buf_sz <= sz) sz = buf_sz;
		buf[sz] = 0;
		memcpy(buf, str, sz);
	}
}

static void utoa(unsigned int val, char *buf, int base)
{
	static char rbuf[16];
	char *ptr = rbuf;

	if(val == 0) {
		*ptr++ = '0';
	}

	while(val) {
		unsigned int digit = val % base;
		*ptr++ = digit < 10 ? (digit + '0') : (digit - 10 + 'a');
		val /= base;
	}

	ptr--;

	while(ptr >= rbuf) {
		*buf++ = *ptr--;
	}
	*buf = 0;
}

static void itoa(int val, char *buf, int base)
{
	static char rbuf[16];
	char *ptr = rbuf;
	int neg = 0;

	if(val < 0) {
		neg = 1;
		val = -val;
	}

	if(val == 0) {
		*ptr++ = '0';
	}

	while(val) {
		int digit = val % base;
		*ptr++ = digit < 10 ? (digit + '0') : (digit - 10 + 'a');
		val /= base;
	}

	if(neg) {
		*ptr++ = '-';
	}

	ptr--;

	while(ptr >= rbuf) {
		*buf++ = *ptr--;
	}
	*buf = 0;
}

