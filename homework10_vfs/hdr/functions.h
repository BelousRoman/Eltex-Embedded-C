/*
* Eltex's academy homework #10 for lecture 31 "Virtual File System"
*
* The first_task() function is a algorithm, using basic file operations, such
* as open, write, syncfs, lseek, read, close and unlink.
*
* The second_task() function is a text editor, written, using ncurses lib, and
* compiled as separate binary.
* Editor is capable of:
* - opening file, if file does not exist - it will be created instead;
* - write and save changes to a file;
* - close file without save;
* Basic interactions are processed by pressing F1, F2 and F3 buttons, pressing
* F1 and F2 will open a popup window for opening and saving the file,
* respectively. These popup windows has buttons, which are navigated through by
* pressing arrow key left and right, Enter key is used to confirm selection.
*/

#ifndef HOMEWORK10_H
#define HOMEWORK10_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <curses.h>

#define T1_FILEPATH "./test.txt"
#define T1_STR_TO_WRITE "Sample text"

/**
 * @brief      Basic file operations
 * @return     Return 0
 */
int first_task();

/**
 * @brief      Simple text editor
 * @return     Return 0
 */
int second_task();


#endif // HOMEWORK10_H
