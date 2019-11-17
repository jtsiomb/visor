#include "vilibc.h"
#include "visor.h"
#include "vimpl.h"

#define vi_malloc(s)	vi->mm.malloc(s)
#define vi_free(p)		vi->mm.free(p)

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

struct vi_buffer *vi_new_buf(struct visor *vi, const char *path)
{
	struct vi_buffer *nb;

	if(!(nb = vi_malloc(sizeof *nb))) {
		vi_error(vi, "failed to allocate new buffer\n");
		return 0;
	}
	memset(nb, 0, sizeof *nb);

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
