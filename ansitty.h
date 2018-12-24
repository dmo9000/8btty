#ifndef __ANSITTY_H__
#define __ANSITTY_H__


int ansitty_init();
int ansitty_putc(unsigned char c);
int ansitty_set_process_fd(int fd);
bool ansitty_setcursorphase(bool cursorstate);
int ansitty_updatecursor();

/* belongs to gfx_opengl.c, but we'll leave it here for now */

int tty_getbuflen();
int input_character();

#endif /* __ANSITTY_H__ */
