#include <cstring>
#include <cassert>
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
pthread_t idle_thread;

void *idle_thread_routine(void *)
{
    while (1) {
        //printf("* IDLE TICK *\n");
        sleep(1);
    }
}

char *envp[] =
{
    "LANG=C",
    "HOME=/home/dan",
    "PATH=/bin:/usr/bin",
    "TZ=UTC0",
    "USER=beelzebub",
    "LOGNAME=tarzan",
    "TERM=ansi",
    "PS1=[\\u@\\h \\W]\\$ ",
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

    pthread_create( &idle_thread, NULL, idle_thread_routine, NULL);

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
    bool ran_input = false;
    bool ran_output = false;
    bool asleep = false;

    while (running) {
        ran_input = false;
        ran_output = false;
        /* main loop */
        //printf("loop ...\n");
        memset(&in_buffer, 0, 2048);
        rd = read(masterFd, &in_buffer, 2047);
        if (rd != -1) {
            //printf("received %d bytes\r\n", rd);
            for (i = 0; i < rd; i++) {
                myTTY->putc(in_buffer[i]);
            }
            ran_input = true;
        } else {
						/* also might get "resource temporarily unavailable" here */
						if (errno == EIO) {
		            perror("pty_read");
								running = false;
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
                ran_input = true;
            } else {
                /* no input waiting */
            }
        }
        if (!ran_input && !ran_output) {
            if (!asleep) {
                //printf("going to sleep!\n");
                asleep = true;
            }

        } else {
            if (asleep) {
                //printf("waking up...\n");
                asleep = false;
            }
        }

        if (asleep) {
            usleep(10000);
            pthread_yield();
        }
    }

    exit(0);
}

