/*
visor - lightweight system-independent text editor and framework
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
typedef void vi_file;

struct visor;
struct vi_buffer;

struct vi_span {
	vi_addr start;
	unsigned long size;
	int src;
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

struct vi_alloc {
	void *(*malloc)(unsigned long);
	void (*free)(void*);
	void *(*realloc)(void*, unsigned long);	/* can be null, will use malloc/free */
};

/* open flags (same as POSIX O_*) */
enum { VI_RDONLY, VI_WRONLY, VI_RDWR, VI_CREAT = 0x100 };
/* seek origin (same as C SEEK_*) */
enum { VI_SEEK_SET, VI_SEEK_CUR, VI_SEEK_END };


struct vi_fileops {
	vi_file *(*open)(const char *path, unsigned int flags);
	void (*close)(vi_file *file);
	long (*size)(vi_file *file);
	void *(*map)(vi_file *file);
	void (*unmap)(vi_file *file);
	long (*read)(vi_file *file, void *buf, long count);
	long (*write)(vi_file *file, void *buf, long count);
	long (*seek)(vi_file *file, long offs, int whence);
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

/* Create a new instance of the visor editor.
 * The alloc argument can be used to provide custom memory allocation
 * functions. It can be null in a hosted build, or if you set the HAVE_LIBC
 * preprocessor macro, in which case the standard library allocator will be
 * used.
 */
struct visor *vi_create(struct vi_alloc *mm);
void vi_destroy(struct visor *vi);

void vi_set_fileops(struct visor *vi, struct vi_fileops *fop);
void vi_set_ttyops(struct visor *vi, struct vi_ttyops *tty);

/* vi_new_buf creates a new buffer and inserts it in the buffer list. If the
 * path pointer is null, the new buffer will be empty, otherwise it's as if it
 * was followed by a vi_buf_read call to read a file into the buffer.
 */
struct vi_buffer *vi_new_buf(struct visor *vi, const char *path);
int vi_delete_buf(struct visor *vi, struct vi_buffer *vb);
int vi_num_buf(struct visor *vi);

struct vi_buffer *vi_getcur_buf(struct visor *vi);
void vi_setcur_buf(struct visor *vi, struct vi_buffer *vb);

struct vi_buffer *vi_next_buf(struct visor *vi);
struct vi_buffer *vi_prev_buf(struct visor *vi);


/* reset a buffer to the newly created state, freeing all resources */
void vi_buf_reset(struct vi_buffer *vb);

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

/* find the span which corresponds to the specified text position
 * if soffs is not null, the relative offset of the specified address from the
 * start of the span is stored there.
 */
struct vi_span *vi_buf_find_span(struct vi_buffer *vb, vi_addr at, vi_addr *soffs);
const char *vi_buf_span_text(struct vi_buffer *vb, struct vi_span *span);

void vi_buf_ins_begin(struct vi_buffer *vb, vi_motion mot);
void vi_buf_insert(struct vi_buffer *vb, char *s);
void vi_buf_ins_end(struct vi_buffer *vb);

void vi_buf_del(struct vi_buffer *vb, vi_motion mot);
void vi_buf_yank(struct vi_buffer *vb, vi_motion mot);

#endif	/* LIB_VISOR_TEXTED_CORE_H_ */
