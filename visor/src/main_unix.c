#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "term.h"
#include "visor.h"

struct file {
	int fd;
	void *maddr;
	size_t msize;
};

static int parse_args(int argc, char **argv);
static int init(void);
static void cleanup(void);
static void resized(int x, int y);
/* file operations */
static vi_file *file_open(const char *path, unsigned int flags);
static void file_close(vi_file *file);
static long file_size(vi_file *file);
static void *file_map(vi_file *file);
static void file_unmap(vi_file *file);
static long file_read(vi_file *file, void *buf, long count);
static long file_write(vi_file *file, void *buf, long count);
static long file_seek(vi_file *file, long offs, int whence);

static struct visor *vi;

static int num_fpaths;
static char **fpaths;

static struct vi_alloc alloc = {
	malloc, free, realloc
};

static struct vi_fileops fops = {
	file_open, file_close, file_size,
	file_map, file_unmap,
	file_read, file_write, file_seek
};

/*
static struct vi_ttyops ttyops = {
	tty_clear, tty_clear_line, tty_clear_line_at,
	tty_setcursor, tty_putchar, tty_putchar_at,
	tty_scroll, tty_del_back, tty_del_fwd, tty_status
};
*/

int main(int argc, char **argv)
{
	if(parse_args(argc, argv) == -1) {
		return 1;
	}
	if(init() == -1) {
		return 1;
	}

	vi_redraw(vi);

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

	if(!(vi = vi_create(&alloc))) {
		return -1;
	}
	vi_set_fileops(vi, &fops);

	for(i=0; i<num_fpaths; i++) {
		if(!vi_new_buf(vi, fpaths[i])) {
			return -1;
		}
	}
	return 0;
}

static void cleanup(void)
{
	if(vi) {
		vi_destroy(vi);
	}
	term_cleanup();
}

static vi_file *file_open(const char *path, unsigned int flags)
{
	struct file *file;

	if(!(file = calloc(1, sizeof *file))) {
		return 0;
	}
	if((file->fd = open(path, flags)) == -1) {
		free(file);
		return 0;
	}
	return (vi_file*)file;
}

static void file_close(vi_file *vif)
{
	struct file *file = vif;
	if(!file) return;

	if(file->fd >= 0) {
		if(file->maddr) {
			file_unmap(file);
		}
		close(file->fd);
	}
	free(file);
}

static long file_size(vi_file *vif)
{
	struct file *file = vif;
	struct stat st;

	if(fstat(file->fd, &st) == -1) {
		return -1;
	}
	return st.st_size;
}

static void *file_map(vi_file *vif)
{
	struct file *file = vif;
	long sz;

	if((sz = file_size(file)) == -1) {
		return 0;
	}
	if((file->maddr = mmap(0, sz, PROT_READ, MAP_PRIVATE, file->fd, 0)) == (void*)-1) {
		return 0;
	}
	file->msize = sz;
	return file->maddr;
}

static void file_unmap(vi_file *vif)
{
	struct file *file = vif;
	if(file->maddr) {
		munmap(file->maddr, file->msize);
	}
	file->maddr = 0;
}

static long file_read(vi_file *vif, void *buf, long count)
{
	struct file *file = vif;
	return read(file->fd, buf, count);
}

static long file_write(vi_file *vif, void *buf, long count)
{
	struct file *file = vif;
	return write(file->fd, buf, count);
}

static long file_seek(vi_file *vif, long offs, int whence)
{
	struct file *file = vif;
	return lseek(file->fd, offs, whence);
}
