/*
* Eltex's academy homework #11 for lecture 32 "Buffered Input/Output"
*
* The first_task() function is a file manager, capable of navigating through
* directories and switching between 2 working windows with TAB button.
* Press F3 to exit.
*/

#ifndef HOMEWORK10_H
#define HOMEWORK10_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <curses.h>
#include <dirent.h>

#define _DEFAULT_SOURCE

#define CWD_SIZE 80

struct window_t {
    int sel_dir;
    int scanned;
    struct dirent **namelist;
    char cwd[CWD_SIZE];
    WINDOW *path_wnd;
    WINDOW *main_wnd;
};

/**
 * @brief      Simple file manager
 * @return     Return 0
 */
int first_task();


#endif // HOMEWORK10_H
