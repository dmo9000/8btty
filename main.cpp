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
		 "LANG=utf-8", 
     "HOME=/home/dan",
     "PATH=/bin:/usr/bin",
     "TZ=UTC0",
     "USER=beelzebub",
     "LOGNAME=tarzan",
		 "TERM=ansi",
		 "PS1=[\\u@\\h \\W]\\$ ",
		// "LS_COLORS=rs=0:di=38;5;33:ln=38;5;51:mh=00:pi=40;38;5;11:so=38;5;13:do=38;5;5:bd=48;5;232;38;5;11:cd=48;5;232;38;5;3:or=48;5;232;38;5;9:mi=01;05;37;41:su=48;5;196;38;5;15:sg=48;5;11;38;5;16:ca=48;5;196;38;5;226:tw=48;5;10;38;5;16:ow=48;5;10;38;5;21:st=48;5;21;38;5;15:ex=38;5;40:*.tar=38;5;9:*.tgz=38;5;9:*.arc=38;5;9:*.arj=38;5;9:*.taz=38;5;9:*.lha=38;5;9:*.lz4=38;5;9:*.lzh=38;5;9:*.lzma=38;5;9:*.tlz=38;5;9:*.txz=38;5;9:*.tzo=38;5;9:*.t7z=38;5;9:*.zip=38;5;9:*.z=38;5;9:*.Z=38;5;9:*.dz=38;5;9:*.gz=38;5;9:*.lrz=38;5;9:*.lz=38;5;9:*.lzo=38;5;9:*.xz=38;5;9:*.zst=38;5;9:*.tzst=38;5;9:*.bz2=38;5;9:*.bz=38;5;9:*.tbz=38;5;9:*.tbz2=38;5;9:*.tz=38;5;9:*.deb=38;5;9:*.rpm=38;5;9:*.jar=38;5;9:*.war=38;5;9:*.ear=38;5;9:*.sar=38;5;9:*.rar=38;5;9:*.alz=38;5;9:*.ace=38;5;9:*.zoo=38;5;9:*.cpio=38;5;9:*.7z=38;5;9:*.rz=38;5;9:*.cab=38;5;9:*.wim=38;5;9:*.swm=38;5;9:*.dwm=38;5;9:*.esd=38;5;9:*.jpg=38;5;13:*.jpeg=38;5;13:*.mjpg=38;5;13:*.mjpeg=38;5;13:*.gif=38;5;13:*.bmp=38;5;13:*.pbm=38;5;13:*.pgm=38;5;13:*.ppm=38;5;13:*.tga=38;5;13:*.xbm=38;5;13:*.xpm=38;5;13:*.tif=38;5;13:*.tiff=38;5;13:*.png=38;5;13:*.svg=38;5;13:*.svgz=38;5;13:*.mng=38;5;13:*.pcx=38;5;13:*.mov=38;5;13:*.mpg=38;5;13:*.mpeg=38;5;13:*.m2v=38;5;13:*.mkv=38;5;13:*.webm=38;5;13:*.ogm=38;5;13:*.mp4=38;5;13:*.m4v=38;5;13:*.mp4v=38;5;13:*.vob=38;5;13:*.qt=38;5;13:*.nuv=38;5;13:*.wmv=38;5;13:*.asf=38;5;13:*.rm=38;5;13:*.rmvb=38;5;13:*.flc=38;5;13:*.avi=38;5;13:*.fli=38;5;13:*.flv=38;5;13:*.gl=38;5;13:*.dl=38;5;13:*.xcf=38;5;13:*.xwd=38;5;13:*.yuv=38;5;13:*.cgm=38;5;13:*.emf=38;5;13:*.ogv=38;5;13:*.ogx=38;5;13:*.aac=38;5;45:*.au=38;5;45:*.flac=38;5;45:*.m4a=38;5;45:*.mid=38;5;45:*.midi=38;5;45:*.mka=38;5;45:*.mp3=38;5;45:*.mpc=38;5;45:*.ogg=38;5;45:*.ra=38;5;45:*.wav=38;5;45:*.oga=38;5;45:*.opus=38;5;45:*.spx=38;5;45:*.xspf=38;5;45:",
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
						printf("going to sleep!\n");
						asleep = true;
						}
				
					} else {
					if (asleep) {
						printf("waking up...\n");
						asleep = false;
						}
					}

				if (asleep) {
						usleep(20000);
						}
    }

    exit(0);
}

