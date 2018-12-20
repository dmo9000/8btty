#ifndef __ANSITTY_H__
#define __ANSITTY_H__


int ansitty_init();
int ansitty_putc(unsigned char c);
int tty_getbuflen();

/* belongs to gfx_opengl.c, but we'll leave it here for now */

int input_character();

#endif /* __ANSITTY_H__ */
