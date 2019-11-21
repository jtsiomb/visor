#ifndef TERM_H_
#define TERM_H_

int term_init(const char *ttypath);
void term_cleanup(void);

void term_clear(void);
int term_getchar(void);

#endif	/* TERM_H_ */
