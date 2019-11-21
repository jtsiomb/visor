#include <stdio.h>
#include "term.h"

static int parse_args(int argc, char **argv);
static int init(void);
static void cleanup(void);

static int num_fpaths;
static char **fpaths;

int main(int argc, char **argv)
{
	if(parse_args(argc, argv) == -1) {
		return 1;
	}
	if(init() == -1) {
		return 1;
	}

	for(;;) {
		int c = term_getchar();

		switch(c) {
		case 27:
		case 'q':
			goto end;
		}
	}
end:

	cleanup();
	return 0;
}

static int parse_args(int argc, char **argv)
{
	int i;

	fpaths = argv + 1;
	num_fpaths = 0;
	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-') {
			fprintf(stderr, "invalid option: %s\n", argv[i]);
			return -1;
		} else {
			argv[++num_fpaths] = argv[i];
		}
	}
	return 0;
}

static int init(void)
{
	int i;

	if(term_init(0) == -1) {
		return -1;
	}
	term_clear();

	for(i=0; i<num_fpaths; i++) {
		/* open fpaths[i] */
	}
	return 0;
}

static void cleanup(void)
{
	term_cleanup();
}
