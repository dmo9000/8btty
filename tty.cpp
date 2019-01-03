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
    TTYDevice = (ANSITTY *) ansitty_init();
    return 1;
}

int TTY::putc(unsigned char c)
{

    ansitty_putc(TTYDevice, c);
    return 1;
}

int TTY::puts(const char *s)
{

    while (s[0] != '\0') {
        putc(s[0]);
        s++;
    }

    return 0;
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
