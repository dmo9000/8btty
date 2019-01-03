#include "object.h"
extern "C" {
#include "ansitty.h"
#include "ansistate.h"
}

class TTY : public Object
{

public:
	TTY();
	~TTY();
	int Init();
	int putc(unsigned char c);
	int puts(const char *s);
	int getchar();
	int hasinput();
	int set_debug(bool debugstate);
		
protected:

private:
	ANSITTY *TTYDevice;

};

