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

	vi_free(vb->path);
	vi_free(vb->orig);
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

static int add_span(struct vi_buffer *vb, int src, vi_addr start, unsigned long size)
{
	struct visor *vi = vb->vi;
	struct vi_span *sp;

	if(vb->num_spans >= vb->max_spans) {
		int newmax = vb->max_spans > 0 ? (vb->max_spans << 1) : 16;
		struct vi_span *tmp = vi_realloc(vb->spans, newmax * sizeof *tmp);
		if(!tmp) return -1;
		vb->spans = tmp;
		vb->max_spans = newmax;
	}

	sp = vb->spans + vb->num_spans++;
	sp->beg = start;
	sp->size = size;
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
}

int vi_buf_read(struct vi_buffer *vb, const char *path)
{
	struct visor *vi = vb->vi;
	vi_file *fp;
	unsigned long fsz;
	int plen;

	vi_buf_reset(vb);

	if(!(fp = vi_open(path))) {
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

		if(add_span(vb, SPAN_ORIG, 0, fsz) == -1) {
			vi_error(vi, "failed to allocate span\n");
			vi_buf_reset(vb);
			return -1;
		}
	}
	vb->orig_size = fsz;
	return 0;
}
