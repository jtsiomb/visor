#ifndef TERM_H_
#define TERM_H_

enum term_event_type {
	TERM_EV_KEY,
	TERM_EV_RESIZE,
	TERM_EV_MBUTTON,
	TERM_EV_MMOTION
};

struct term_ev_any {
	int type;
};

struct term_ev_resize {
	int type;
	int width, height;
};

struct term_ev_mbutton {
	int type;
	/* TODO */
};

struct term_ev_mmotion {
	int type;
	int x, y;
};

union term_event {
	struct term_ev_any any;
	struct term_ev_resize resize;
	struct term_ev_mbutton mbutton;
	struct term_ev_mmotion mmotion;
};

int term_init(const char *ttypath);
void term_cleanup(void);

void term_getsize(int *width, int *height);
void term_resize_func(void (*func)(int, int));

void term_send(const char *s, int size);
void term_putchar(char c);
void term_puts(const char *s);
void term_printf(const char *fmt, ...);
void term_flush(void);

void term_clear(void);
void term_setcursor(int row, int col);

int term_getchar(void);

#endif	/* TERM_H_ */
