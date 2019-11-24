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
#ifndef VIMPL_H_
#define VIMPL_H_

#include "visor.h"

struct visor {
	struct vi_fileops fop;
	struct vi_buffer *buflist;	/* circular linked list of buffers cur first */
	struct vi_alloc mm;
	struct vi_ttyops tty;
	void *tty_cls;

	int term_width, term_height;
};

struct vi_buffer {
	struct visor *vi;
	char *path;
	struct vi_buffer *next, *prev;

	vi_addr cursor, view_start;
	int view_xscroll;

	vi_file *fp;
	int file_mapped;

	char *orig;
	unsigned long orig_size;
	char *add;
	int add_size, add_max;

	struct vi_span *spans;
	int num_spans, max_spans;
};

enum { SPAN_ORIG, SPAN_ADD };

#endif	/* VIMPL_H_ */
