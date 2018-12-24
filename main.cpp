#include <cstring>
#include <cassert>
#include <iostream>
#include <pty.h>
#include "tty.h"
#include "unistd.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>


using std::cout;
using std::endl;
using std::memset;
pthread_t idle_thread;
uint8_t cursor_count;

void *idle_thread_routine(void *)
{
    while (1) {
        //printf("* IDLE TICK *\n");
        //sleep(1);
        usleep(750000);
        /* this will wrap around, but that's ok */
        cursor_count++;
        if (cursor_count % 2) {
            //fprintf(stderr, "CURSOR OFF\n");
            ansitty_setcursorphase(false);
            ansitty_updatecursor();
        } else {
            //fprintf(stderr, "CURSOR ON\n");
            ansitty_setcursorphase(true);
            ansitty_updatecursor();
        }
    }
}

char *envp[] =
{
    "LANG=C",
    "DISPLAY=:0",
    "HOME=/home/dan",
    "PATH=/bin:/usr/bin",
    "TZ=UTC0",
    "USER=beelzebub",
    "LOGNAME=tarzan",
    "TERM=ansi",
    "PS1=[\\u@\\h \\W]\\$ ",
    0
};

static void
change_term_size (int fd, int x, int y)
{

    struct winsize win;
    printf("1) change_term_size(%d, %u, %u)\n", fd, x, y);
    if (ioctl (fd, TIOCGWINSZ, &win))
        return;
    if (y && y >24)
        win.ws_row = y;
    else
        win.ws_row = 24;
    if (x && x>80)
        win.ws_col = x;
    else
        win.ws_col = 80;
    ioctl (fd, TIOCSWINSZ, &win);
    printf("2) change_term_size(%u, %u)\n", win.ws_col, win.ws_row);
}


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

    /* see here for notes on resizing:
    		https://www.ohse.de/uwe/software/resize.c.html */

    pthread_create( &idle_thread, NULL, idle_thread_routine, NULL);

    int masterFd;
    char* args[] = {"/bin/bash", "-i", NULL };
    //char* args[] = {"/bin/bash",  NULL };
    setenv("TERM", "ansi", 1);
    int procId = forkpty(&masterFd, NULL, NULL,  NULL);
    if( procId == 0 ) {
        setenv("TERM", "ansi", 1);
        //change_term_size(0, 80, 24);
        execve( args[0], args, envp);
        /* TODO error handling if that didn't work */
    }

    //myTTY->puts("(in parent process...)\n");

    printf("child pid = %u\r\n", procId);
    int flags = fcntl(masterFd, F_GETFL, 0);
    fcntl(masterFd, F_SETFL, flags | O_NONBLOCK);

    change_term_size(masterFd, 80, 24);

    ansitty_set_process_fd(masterFd);
    running = true;

    ssize_t rd = 0;
    int i = 0;
    int j = 0;
    bool ran_input = false;
    bool ran_output = false;
    bool asleep = false;

    //ansi_setdebug(true);

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
                assert(NULL);
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

