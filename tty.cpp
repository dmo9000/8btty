#include "tty.h"
#include "ansitty.h"

TTY::TTY()
{

	std::cout << "TTY created" << std::endl;


}

TTY::~TTY()
{

	std::cout << "TTY destroyed" << std::endl;

}

int TTY::Init()
{

	std::cout << "TTY::Init()" << std::endl;
	ansitty_init();

}

int TTY::putc(unsigned char c)
{

	ansitty_putc(c);
	return 0;

}

int TTY::puts(const char *s)
{

	while (s[0] != '\0') {
			putc(s[0]);
			s++;
			}

}

int TTY::getchar()
{

	return input_character();

}

int TTY::hasinput()
{
	return tty_getbuflen();
}

int TTY::set_debug(bool debugstate)
{
	return ansi_setdebug(debugstate);

}
