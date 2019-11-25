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

#include "vilibc.h"
#include "visor.h"
#include "vimpl.h"

#define vi_malloc	vi->mm.malloc
#define vi_free		vi->mm.free
#define vi_realloc	vi->mm.realloc

#define vi_open		vi->fop.open
#define vi_size		vi->fop.size
#define vi_close	vi->fop.close
#define vi_map		vi->fop.map
#define vi_unmap	vi->fop.unmap
#define vi_read		vi->fop.read
#define vi_write	vi->fop.write
#define vi_seek		vi->fop.seek

#define vi_clear()			vi->tty.clear(vi->tty_cls)
#define vi_clear_line()		vi->tty.clear_line(vi->tty_cls)
#define vi_clear_line_at(y)	vi->tty.clear_line_at(y, vi->tty_cls)
#define vi_setcursor(x, y)	vi->tty.setcursor(x, y, vi->tty_cls)
#define vi_putchar(c)		vi->tty.putchar(c, vi->tty_cls)
#define vi_putchar_at(x, y, c)	v->tty.putchar_at(x, y, c, vi->tty_cls)
#define vi_scroll(n)		vi->tty.scroll(n, vi->tty_cls)
#define vi_del_back()		vi->tty.del_back(vi->tty_cls)
#define vi_del_fwd()		vi->tty.del_fwd(vi->tty_cls)
#define vi_status(s)		vi->tty.status(s, vi->tty_cls)
#define vi_flush()			vi->tty.flush(vi->tty_cls)

static int remove_buf(struct visor *vi, struct vi_buffer *vb);
static int add_span(struct vi_buffer *vb, vi_addr at, int src, vi_addr start, unsigned long size);

#ifdef HAVE_LIBC
static const struct vi_alloc stdalloc = { malloc, free, realloc };
#endif

struct visor *vi_create(struct vi_alloc *mm)
{
	struct visor *vi;

#ifdef HAVE_LIBC
	if(!mm) mm = &stdalloc;
#else
	if(!mm) return 0;
#endif

	if(!(vi = mm->malloc(sizeof *vi))) {
		return 0;
	}
	memset(vi, 0, sizeof *vi);
	vi->mm = *mm;

	vi->term_width = 80;
	vi->term_height = 24;

	return vi;
}

void vi_destroy(struct visor *vi)
{
	while(vi->buflist) {
		vi_delete_buf(vi, vi->buflist);
	}
	vi_free(vi);
}

void vi_set_fileops(struct visor *vi, struct vi_fileops *fop)
{
	vi->fop = *fop;
}

void vi_set_ttyops(struct visor *vi, struct vi_ttyops *tty)
{
	vi->tty = *tty;
}

void vi_term_size(struct visor *vi, int xsz, int ysz)
{
	vi->term_width = xsz;
	vi->term_height = ysz;
}

void vi_redraw(struct visor *vi)
{
	int i, col, cur_x = 0, cur_y = 0;
	char c;
	struct vi_buffer *vb;
	struct vi_span *sp, *spans_end;
	const char *tptr, *tend;
	vi_addr spoffs, addr;

	vb = vi->buflist;
	if(!(sp = vi_buf_find_span(vb, vb->view_start, &spoffs))) {
		sp = vb->spans;
		spoffs = 0;
	}
	spans_end = vb->spans + vb->num_spans;

	tptr = vi_buf_span_text(vb, sp);
	tend = tptr + sp->size;
	tptr += spoffs;

	vi_clear();

	addr = vb->view_start;
	for(i=0; i<vi->term_height; i++) {
		vi_setcursor(0, i);
		col = -vb->view_xscroll - 1;
		while(++col < vi->term_width && (c = (addr++, *tptr++)) != '\n') {
			if(addr == vb->cursor) {
				cur_x = col;
				cur_y = i;
			}

			if(col >= 0) {
				vi_putchar(c);
			}
			if(tptr >= tend) {
				if(++sp >= spans_end) {
					goto end;
				}
				tptr = vi_buf_span_text(vb, sp);
				tend = tptr + sp->size;
			}
		}
	}
end:

	while(i < vi->term_height) {
		vi_setcursor(0, i++);
		vi_putchar('~');
	}

	vi_setcursor(cur_x, cur_y);
	vi_flush();
}

struct vi_buffer *vi_new_buf(struct visor *vi, const char *path)
{
	struct vi_buffer *nb;

	if(!(nb = vi_malloc(sizeof *nb))) {
		vi_error(vi, "failed to allocate new buffer\n");
		return 0;
	}
	memset(nb, 0, sizeof *nb);
	nb->vi = vi;

	if(path) {
		if(vi_buf_read(nb, path) == -1) {
			vi_free(nb);
			return 0;
		}
	}

	if(vi->buflist) {
		struct vi_buffer *last = vi->buflist->prev;
		nb->prev = last;
		nb->next = vi->buflist;
		last->next = nb;
		vi->buflist->prev = nb;
	} else {
		nb->next = nb->prev = nb;
		vi->buflist = nb;
	}
	return nb;
}

static int remove_buf(struct visor *vi, struct vi_buffer *vb)
{
	if(!vi->buflist) {
		vi_error(vi, "failed to remove a buffer which doesn't exist\n");
		return -1;
	}

	if(vb->next == vb) {
		if(vi->buflist != vb) {
			vi_error(vi, "failed to remove buffer, buffer list inconsistency\n");
			return -1;
		}
		vi->buflist = 0;
		return 0;
	}

	if(vi->buflist == vb) {
		vi->buflist = vb->next;
	}
	vb->prev->next = vb->next;
	vb->next->prev = vb->prev;
	vb->next = vb->prev = vb;
	return 0;
}

int vi_delete_buf(struct visor *vi, struct vi_buffer *vb)
{
	if(remove_buf(vi, vb) == -1) {
		return -1;
	}

	if(vb->fp) {
		if(vb->file_mapped) {
			vi_unmap(vb->fp);
		} else {
			vi_free(vb->orig);
		}
		vi_close(vb->fp);
	}

	vi_free(vb->path);
	vi_free(vb->add);
	vi_free(vb->spans);
	return 0;
}

int vi_num_buf(struct visor *vi)
{
	int count;
	struct vi_buffer *vb;

	if(!vi->buflist) return 0;

	count = 1;
	vb = vi->buflist->next;
	while(vb != vi->buflist) {
		count++;
		vb = vb->next;
	}
	return count;
}

struct vi_buffer *vi_getcur_buf(struct visor *vi)
{
	return vi->buflist;
}

void vi_setcur_buf(struct visor *vi, struct vi_buffer *vb)
{
	vi->buflist = vb;
}

struct vi_buffer *vi_next_buf(struct visor *vi)
{
	return vi->buflist ? vi->buflist->next : 0;
}

struct vi_buffer *vi_prev_buf(struct visor *vi)
{
	return vi->buflist ? vi->buflist->prev : 0;
}

/* split_span splits the span sp. if size > 0 it moves the second part to sp+2,
 * leaving an empty place at sp+1 for the new span. The start point of the
 * second part is adjusted by size.
 *
 * It can't fail, because it's always called with the span array having at
 * least two empty slots (see: add_span).
 */
void split_span(struct vi_buffer *vb, struct vi_span *sp, vi_addr spoffs, unsigned long size)
{
	int newseg = size > 0 ? 1 : 0;
	struct vi_span *tail = sp + newseg + 1;
	int num_move = vb->spans + vb->num_spans - sp - 1;

	memmove(tail + 1, sp + 1, num_move * sizeof *sp);
	vb->num_spans += tail - sp;

	*tail = *sp;
	sp->size = spoffs;
	tail->start += spoffs;
	tail->size -= spoffs;

	sp = tail;
	for(;;) {
		if(size <= tail->size) {
			tail->size -= size;
			break;
		}
		size -= tail->size;
		tail->size = 0;
		tail++;
	}

	if(tail > sp) {
		/* we produced one or more zero-sized spans, drop them */
		num_move = vb->num_spans - (tail - sp);
		memmove(sp, tail, num_move * sizeof *sp);
		vb->num_spans -= num_move;
	}
}

static int add_span(struct vi_buffer *vb, vi_addr at, int src, vi_addr start, unsigned long size)
{
	struct visor *vi = vb->vi;
	struct vi_span *sp;
	vi_addr spoffs;

	/* make sure we have space for at least two new spans (split + add) */
	if(vb->num_spans + 1 >= vb->max_spans) {
		int newmax = vb->max_spans > 0 ? (vb->max_spans << 1) : 16;
		struct vi_span *tmp = vi_realloc(vb->spans, newmax * sizeof *tmp);
		if(!tmp) return -1;
		vb->spans = tmp;
		vb->max_spans = newmax;
	}

	if((sp = vi_buf_find_span(vb, at, &spoffs))) {
		if(spoffs > 0) {
			split_span(vb, sp++, spoffs, 1);
		} else {
			split_span(vb, sp++, 0, 0);
		}
	} else {
		sp = vb->spans + vb->num_spans;
	}

	sp->src = src;
	sp->start = start;
	sp->size = size;
	vb->num_spans++;
	return 0;
}

void vi_buf_reset(struct vi_buffer *vb)
{
	struct visor *vi = vb->vi;
	struct vi_buffer *prev, *next;

	vi_free(vb->path);

	if(vb->fp) {
		if(vb->file_mapped) vi_unmap(vb->fp);
		vi_close(vb->fp);
	}
	vi_free(vb->orig);
	vi_free(vb->add);
	vi_free(vb->spans);

	prev = vb->prev;
	next = vb->next;
	memset(vb, 0, sizeof *vb);
	vb->prev = prev;
	vb->next = next;
	vb->vi = vi;
}

int vi_buf_read(struct vi_buffer *vb, const char *path)
{
	struct visor *vi = vb->vi;
	vi_file *fp;
	unsigned long fsz;
	int plen;

	vi_buf_reset(vb);

	if(!(fp = vi_open(path, VI_RDONLY | VI_CREAT))) {
		return -1;
	}
	plen = strlen(path);
	if(!(vb->path = vi_malloc(plen + 1))) {
		vi_error(vi, "failed to allocate path name buffer\n");
		vi_buf_reset(vb);
		return -1;
	}
	memcpy(vb->path, path, plen + 1);

	vb->num_spans = 0;

	if((fsz = vi_size(fp))) {
		/* existing file, map it into memory, or failing that read it */
		if(!vi->fop.map || !(vb->orig = vi_map(fp))) {
			if(!(vb->orig = vi_malloc(fsz))) {
				vi_buf_reset(vb);
				return -1;
			}
		} else {
			vb->file_mapped = 1;
		}

		if(add_span(vb, 0, SPAN_ORIG, 0, fsz) == -1) {
			vi_error(vi, "failed to allocate span\n");
			vi_buf_reset(vb);
			return -1;
		}
	}
	vb->orig_size = fsz;
	return 0;
}

int vi_buf_write(struct vi_buffer *vb, const char *path)
{
	int i, wbuf_count;
	struct visor *vi = vb->vi;
	vi_file *fp;
	static char wbuf[512];

	if(!path) path = vb->path;
	if(!path) {
		vi_error(vi, "failed to write buffer, unknown path\n");
		return -1;
	}

	if(!(fp = vi_open(path, VI_WRONLY | VI_CREAT))) {
		vi_error(vi, "failed to open %s for writing\n", path);
		return -1;
	}

	wbuf_count = 0;
	for(i=0; i<vb->num_spans; i++) {
		struct vi_span *sp = vb->spans + i;
		const char *sptxt = vi_buf_span_text(vb, sp);
		int n, count = 0;
		while(count < sp->size) {
			n = sp->size - count;
			if(n > sizeof wbuf - wbuf_count) {
				n = sizeof wbuf - wbuf_count;
			}
			memcpy(wbuf + wbuf_count, sptxt + count, n);
			count += n;
			wbuf_count += n;
		}

		if(wbuf_count >= sizeof wbuf) {
			vi_write(fp, wbuf, wbuf_count);
		}
	}

	if(wbuf_count > 0) {
		vi_write(fp, wbuf, wbuf_count);
	}
	vi_close(fp);
	return 0;
}

long vi_buf_size(struct vi_buffer *vb)
{
	int i;
	long sz = 0;

	for(i=0; i<vb->num_spans; i++) {
		sz += vb->spans[i].size;
	}
	return sz;
}

struct vi_span *vi_buf_find_span(struct vi_buffer *vb, vi_addr at, vi_addr *soffs)
{
	int i;
	long sz = 0, prev_sz;

	for(i=0; i<vb->num_spans; i++) {
		prev_sz = sz;
		sz += vb->spans[i].size;
		if(sz > at) {
			if(soffs) *soffs = at - prev_sz;
			return vb->spans + i;
		}
	}
	return 0;
}

const char *vi_buf_span_text(struct vi_buffer *vb, struct vi_span *sp)
{
	const char *buf = sp->src == SPAN_ORIG ? vb->orig : vb->add;
	return buf + sp->start;
}
