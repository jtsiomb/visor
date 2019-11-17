#ifndef VIMPL_H_
#define VIMPL_H_

#include "visor.h"

struct visor {
	struct vi_fileops fop;
	struct vi_buffer *buflist;	/* circular linked list of buffers cur first */
	struct vi_alloc mm;
	struct vi_ttyops tty;
	void *tty_cls;
};

struct vi_buffer {
	struct visor *vi;
	char *path;
	struct vi_buffer *next, *prev;

	vi_addr cursor, view_start;
	int view_xscroll;

	char *orig;
	long orig_size;
	char *add;
	int add_size, add_max;

	struct vi_span *spans;
	int num_spans, max_spans;
};

#endif	/* VIMPL_H_ */
