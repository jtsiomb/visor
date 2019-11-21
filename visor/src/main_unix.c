#include <stdio.h>

static int init(void);
static void cleanup(void);
static void sighandler(int s);

int main(int argc, char **argv)
{
	int res;
	char c;

	if(init() == -1) {
		return 1;
	}

	for(;;) {
		if(term_getchar() == 'q') {
			break;
		}
		/* proc input */
	}

	cleanup();
	return 0;
}

static int init(void)
{
	if(term_init() == -1) {
		return -1;
	}
	term_clear();
	return 0;
}

static void cleanup(void)
{
	term_cleanup();
}
