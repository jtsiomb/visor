#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "term.h"

static void sighandler(int s);

static int term_width, term_height;
static int ttyfd = -1;
static int selfpipe[2];
static struct termios saved_term;

static void (*cb_resized)(int, int);


int term_init(const char *ttypath)
{
	struct termios term;
	struct winsize winsz;

	if((ttyfd = open(ttypath ? ttypath : "/dev/tty", O_RDWR)) == -1) {
		perror("failed to open /dev/tty");
		return -1;
	}
	if(tcgetattr(ttyfd, &term) == -1) {
		perror("failed to get terminal attr");
		return -1;
	}
	saved_term = term;
	term.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	term.c_oflag &= ~OPOST;
	term.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	term.c_cflag = (term.c_cflag & ~(CSIZE | PARENB)) | CS8;

	if(tcsetattr(ttyfd, TCSAFLUSH, &term) == -1) {
		perror("failed to change terminal attributes");
		return -1;
	}

	ioctl(1, TIOCGWINSZ, &winsz);
	term_width = winsz.ws_col;
	term_height = winsz.ws_row;

	pipe(selfpipe);

	signal(SIGWINCH, sighandler);
	return 0;
}

void term_cleanup(void)
{
	term_clear();
	term_setcursor(0, 0);
	term_flush();
	tcsetattr(ttyfd, TCSAFLUSH, &saved_term);
	close(ttyfd);
	ttyfd = -1;
}

void term_reset(void)
{
	term_puts("\033c");
	term_flush();
}

void term_getsize(int *width, int *height)
{
	*width = term_width;
	*height = term_height;
}

void term_resize_func(void (*func)(int, int))
{
	cb_resized = func;
}


static char termbuf[1024];
static int termbuf_len;

void term_send(const char *s, int size)
{
	if(size >= sizeof termbuf) {
		/* too large, just flush the buffer and write directly to the tty */
		term_flush();
		write(ttyfd, s, size);
	} else {
		if(size >= sizeof termbuf - termbuf_len) {
			term_flush();
		}
		memcpy(termbuf + termbuf_len, s, size);
		termbuf_len += size;
	}
}

void term_putchar(char c)
{
	term_send(&c, 1);
}

void term_puts(const char *s)
{
	term_send(s, strlen(s));
}

void term_printf(const char *fmt, ...)
{
	static char *buf;
	static long bufsz;
	va_list ap;
	long len;

	if(!buf) {
		bufsz = 512;
		if(!(buf = malloc(bufsz))) {
			return;
		}
	}

	for(;;) {
		va_start(ap, fmt);
		len = vsnprintf(buf, bufsz, fmt, ap);
		va_end(ap);

		if(len < bufsz) break;
		if(len < 0) {
			void *tmp;
			long n = bufsz << 1;
			if(!(tmp = realloc(buf, n))) {
				break;	/* if realloc fails, will result in truncated output */
			}
		}
	}

	term_send(buf, len);
}

void term_flush(void)
{
	if(termbuf_len > 0) {
		write(ttyfd, termbuf, termbuf_len);
		termbuf_len = 0;
	}
}

void term_clear(void)
{
	term_puts("\033[2J");
}

void term_cursor(int show)
{
	term_printf("\033[?25%c", show ? 'h' : 'l');
}

void term_setcursor(int row, int col)
{
	term_printf("\033[%d;%dH", row + 1, col + 1);
}

int term_getchar(void)
{
	int res;
	char c;
	while((res = read(ttyfd, &c, 1)) < 0 && errno == EINTR);
	if(res <= 0) return -1;
	return c;
}


static void sighandler(int s)
{
	struct winsize winsz;

	signal(s, sighandler);

	switch(s) {
	case SIGWINCH:
		ioctl(1, TIOCGWINSZ, &winsz);
		term_width = winsz.ws_col;
		term_height = winsz.ws_row;
		/* redraw */
		break;

	default:
		break;
	}
}
