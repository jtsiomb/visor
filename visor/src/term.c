#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "term.h"

static void sighandler(int s);

static int term_width, term_height;
static int ttyfd = -1;
static struct termios saved_term;


int term_init(const char *ttypath)
{
	struct termios term;
	struct winsize winsz;

	if((ttyfd = open("/dev/tty", O_RDWR)) == -1) {
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

	signal(SIGWINCH, sighandler);
	return 0;
}

void term_cleanup(void)
{
	tcsetattr(ttyfd, TCSAFLUSH, &saved_term);
	close(ttyfd);
	ttyfd = -1;
}

void term_clear(void)
{
	write(ttyfd, "\033[2J", 4);
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
