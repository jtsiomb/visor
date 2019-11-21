#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>

static int init(void);
static void cleanup(void);
static void sighandler(int s);

int term_width, term_height;
int ttyfd;
struct termios saved_term;

int main(int argc, char **argv)
{
	int res;
	char c;

	if(init() == -1) {
		return 1;
	}

	for(;;) {
		if((res = read(ttyfd, &c, 1)) == 0 || (res < 0 && errno != EINTR)) {
			break;
		}
		/* proc input */
	}

	cleanup();
	return 0;
}

static int init(void)
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

	write(ttyfd, "\033[2J", 4);

	return 0;
}

static void cleanup(void)
{
	tcsetattr(ttyfd, TCSAFLUSH, &saved_term);
	close(ttyfd);
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
