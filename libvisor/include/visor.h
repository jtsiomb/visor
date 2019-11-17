/*
This file is part of the visor text editor and text editor framework
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
#ifndef LIB_VISOR_TEXTED_CORE_H_
#define LIB_VISOR_TEXTED_CORE_H_

typedef long vi_addr;
typedef long vi_motion;

struct visor;
struct vi_buffer;

struct vi_span {
	vi_addr beg, end;
};

enum vi_motdir {
	VI_MOT_LEFT			= 'h',
	VI_MOT_DOWN			= 'j',
	VI_MOT_UP			= 'k',
	VI_MOT_RIGHT		= 'l',
	VI_MOT_WORD_NEXT	= 'w',
	VI_MOT_WORD_END		= 'e',
	VI_MOT_WORD_BEG		= 'b',
	VI_MOT_WORDP_NEXT	= 'W',
	VI_MOT_WORDP_BEG	= 'B',
	VI_MOT_LINE_BEG		= '^',
	VI_MOT_LINE_END		= '$',
	VI_MOT_SENT_NEXT	= ')',
	VI_MOT_SENT_PREV	= '(',
	VI_MOT_PAR_NEXT		= '}',
	VI_MOT_PAR_PREV		= '{',
	VI_MOT_SECT_NEXT	= ']',
	VI_MOT_SECT_PREV	= '[',
	VI_MOT_FIND_NEXT	= 'f',
	VI_MOT_FIND_PREV	= 'F',
	VI_MOT_FINDTO_NEXT	= 't',
	VI_MOT_FINDTO_PREV	= 'T',
	VI_MOT_GO			= 'G',
	VI_MOT_TOP			= 'H',
	VI_MOT_MID			= 'M',
	VI_MOT_BOT			= 'B',
	VI_MOT_INNER		= 'i',
	VI_MOT_OUTER		= 'a'
};

#define VI_MOTION(d, n) (((long)(n) << 8) | ((long)(d)))


struct vi_fileops {
	void *(*open)(const char *path);
	long (*size)(void *file);
	void (*close)(void *file);
	void *(*map)(void *file);
	void (*unmap)(void *file);
	long (*read)(void *file, void *buf, long count);
	long (*write)(void *file, void *buf, long count);
};

struct vi_ttyops {
	void (*clear)(void *cls);
	void (*clear_line)(void *cls);
	void (*clear_line_at)(int y, void *cls);
	void (*setcursor)(int x, int y, void *cls);
	void (*putchar)(char c, void *cls);
	void (*putchar_at)(int x, int y, char c, void *cls);
	void (*scroll)(int nlines, void *cls);
	void (*del_back)(void *cls);
	void (*del_fwd)(void *cls);
	void (*status)(char *s, void *cls);
};


struct visor *vi_init(void);
void vi_cleanup(struct visor *vi);

void vi_set_fileops(struct visor *vi, struct vi_fileops *fop);

/* vi_new_buf creates a new buffer and inserts it in the buffer list. If the
 * path pointer is null, the new buffer will be empty, otherwise it's as if it
 * was followed by a vi_buf_read call to read a file into the buffer.
 */
struct vi_buffer *vi_new_buf(struct visor *vi, const char *path);
int vi_delete_buf(struct visor *vi, struct vi_buffer *vb);
int vi_num_buf(struct visor *vi);

struct vi_buffer *vi_getcur_buf(struct visor *vi);
int vi_setcur_buf(struct visor *vi, struct vi_buffer *vb);

struct vi_buffer *vi_next_buf(struct visor *vi);
struct vi_buffer *vi_prev_buf(struct visor *vi);


/* Read a file into the buffer.
 * Returns 0 on success, -1 on failure.
 */
int vi_buf_read(struct vi_buffer *vb, const char *path);

/* Write the buffer out to a file. If the path is null, the buffer will be
 * written out to the same file that was last read. If the path is null and
 * no file was ever read in this buffer, the write fails.
 * Returns 0 on success, -1 on failure.
 */
int vi_buf_write(struct vi_buffer *vb, const char *path);
long vi_buf_size(struct vi_buffer *vb);

void vi_buf_ins_begin(struct vi_buffer *vb, vi_motion mot);
void vi_buf_insert(struct vi_buffer *vb, char *s);
void vi_buf_ins_end(struct vi_buffer *vb);

void vi_buf_del(struct vi_buffer *vb, vi_motion mot);
void vi_buf_yank(struct vi_buffer *vb, vi_motion mot);

#endif	/* LIB_VISOR_TEXTED_CORE_H_ */
