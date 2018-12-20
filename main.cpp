#include <cstring>
#include <iostream>
#include <pty.h>
//#include "actor.h"
#include "tty.h"
#include "unistd.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>


using std::cout;
using std::endl;
using std::memset;

char *envp[] =
{
     "HOME=/",
     "PATH=/bin:/usr/bin",
     "TZ=UTC0",
     "USER=beelzebub",
     "LOGNAME=tarzan",
		 "TERM=ansi",
     0
    };


int main(int argc, char *argv[])
{

    TTY *myTTY = NULL;
    char in_buffer[2048];
    char out_buffer[2048];
    int bytes =0;
    bool running = false;
    char c = 0;

    myTTY = new TTY();
    myTTY->Init();

    sleep(1);

    int masterFd;
    char* args[] = {"/bin/bash", "-i", NULL };
    //char* args[] = {"/bin/bash",  NULL };
    setenv("TERM", "ansi", 1);
    int procId = forkpty(&masterFd, NULL, NULL,  NULL);
    if( procId == 0 ) {
        setenv("TERM", "ansi", 1);
        //myTTY->puts("(in child process...)\n");
     //   fprintf(stderr, "(in child process)\r\n");
        execve( args[0], args, envp);
        /* TODO error handling if that didn't work */
    }

    //myTTY->puts("(in parent process...)\n");

    printf("child pid = %u\r\n", procId);
    int flags = fcntl(masterFd, F_GETFL, 0);
    fcntl(masterFd, F_SETFL, flags | O_NONBLOCK);

    running = true;

    ssize_t rd = 0;
    int i = 0;
		int j = 0;

    while (running) {
        /* main loop */
        //printf("loop ...\n");
        memset(&in_buffer, 0, 2048);
        rd = read(masterFd, &in_buffer, 2047);
        if (rd != -1) {
            //printf("received %d bytes\r\n", rd);
            for (i = 0; i < rd; i++) {
                myTTY->putc(in_buffer[i]);
            }
        }

				//printf("buflen = %u\n", j);
				j = tty_getbuflen();
        if (j) {
            c = myTTY->getchar();
						//printf("c = %u\n", c);
            if(c != 0) {
                out_buffer[1] = '\0';
                out_buffer[0] = c;
                write(masterFd, &out_buffer, 1);
            } else {
                /* no input waiting */
            }
        }
    }

    exit(0);
}

