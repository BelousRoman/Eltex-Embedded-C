#include "../hdr/functions.h"

/* Global variable, used to store terminal's size in columns and rows */
struct winsize size;

/* Function to process signal, called on resizing terminal window */
void sig_winch(int signo)
{
	ioctl(fileno(stdout), TIOCGWINSZ, (char *) &size);
	resizeterm(size.ws_row, size.ws_col);
}

int first_task()
{
	puts("First task");

	/*
	* Declare:
	* - fd - file descriptor;
	* - buf - buffer to store symbol read from fd.
	*/
	int fd;
	char buf;

	/* Create and open file to read and write */
	fd = open(T1_FILEPATH, O_CREAT | O_RDWR, 0666);

	/* Write string into a file */
	write(fd, T1_STR_TO_WRITE, strlen(T1_STR_TO_WRITE));

	/* Write all the data from buffer to file */
	syncfs(fd);

	/* Set the cursor to the start of file */
	lseek(fd, 0, 0);

	/* Read the file until EOF */
	while(read(fd, &buf, 1) > 0)
	{
		write(STDOUT_FILENO, &buf, 1);
	}
	write(STDOUT_FILENO, "\n", 1);

	/* Close fd */
	close(fd);

	/* Delete the file */
	unlink(T1_FILEPATH);

	return 0;
}

/* Wrapper to create a new window */
int _new_window(WINDOW **wnd, WINDOW **subwnd, int ys, int xs, int yp, int xp, char v, char h)
{
	*wnd = newwin(ys, xs, yp, xp);
	box(*wnd, v, h);
	*subwnd = derwin(*wnd, (ys-2), (xs-2), 1, 1);

	return EXIT_SUCCESS;
}

/* Wrapper to create a window, using derwin() from pwnd */
int _create_window(WINDOW **pwnd, WINDOW **wnd, WINDOW **subwnd, int ys, int xs,
					int yp, int xp, char v, char h)
{
	*wnd = derwin(*pwnd, ys, xs, yp, xp);
	box(*wnd, v, h);
	*subwnd = derwin(*wnd, (ys-2), (xs-2), 1, 1);

	return EXIT_SUCCESS;
}

/* Wrapper to create window, asking user which file to open */
int _fopen_window(WINDOW **pwnd, WINDOW **wnd, WINDOW **subwnd,
				WINDOW **text_wnd, WINDOW **text_subwnd,
				WINDOW **confirm_wnd, WINDOW **confirm_subwnd,
				WINDOW **cancel_wnd, WINDOW **cancel_subwnd, FILE **file,
				char *fname)
{
	/*
	* Declare:
	* - symbol - variable, storing return value of wgetch() function;
	* - index - variable, storing current index of fname char array;
	* - ret - return value of function, also used to process confirm and cancel
	* buttons.
	*/
	int symbol;
	int index = 0;
	int ret = 0;

	curs_set(0);
	
	/* Call _create_window wrapper to create windows and fill them with info */
	_create_window(pwnd, wnd, subwnd, 10, 50, 5, 40, '|', '-');
	wmove(*subwnd, 0, (50/2 - strlen("Enter name of the file")/2));
	wprintw(*subwnd, "Enter name of the file");
	wbkgd(*wnd, COLOR_PAIR(2));
	_create_window(subwnd, text_wnd, text_subwnd, 3, 46, 2, 1, '|', '-');
	_create_window(subwnd, confirm_wnd, confirm_subwnd, 3, 20, 5, 1, ' ', ' ');
	_create_window(subwnd, cancel_wnd, cancel_subwnd, 3, 20, 5, 27, ' ', ' ');
	wmove(*confirm_subwnd, 0, (20/2 - strlen("Confirm")/2 - 1));
	wprintw(*confirm_subwnd, "Confirm");
	wmove(*cancel_subwnd, 0, (20/2 - strlen("Cancel")/2 - 1));
	wprintw(*cancel_subwnd, "Cancel");

	/*
	* Set background color to usual color palette to every window, except
	* for the confirm window, mark it as selected option.
	*/
	wbkgd(*wnd, COLOR_PAIR(1));
	wbkgd(*subwnd, COLOR_PAIR(1));
	wbkgd(*text_wnd, COLOR_PAIR(1));
	wbkgd(*text_subwnd, COLOR_PAIR(1));
	wbkgd(*confirm_wnd, COLOR_PAIR(2));
	wbkgd(*cancel_wnd, COLOR_PAIR(1));
	
	wrefresh(*wnd);

	/*
	* Read user input to either write chars to fname or process left, right
	* arrows and enter keys.
	*/
	keypad(*text_subwnd, true);
	while(1)
	{
		symbol = wgetch(*text_subwnd);
		if ('\n' == symbol)
		{
			break;
		}
		else if (symbol == 260)
		{
			ret = 0;
			wbkgd(*confirm_wnd, COLOR_PAIR(2));
			wbkgd(*cancel_wnd, COLOR_PAIR(1));
			wrefresh(*confirm_wnd);
			wrefresh(*cancel_wnd);
		}
		else if (symbol == 261)
		{
			ret = -1;
			wbkgd(*confirm_wnd, COLOR_PAIR(1));
			wbkgd(*cancel_wnd, COLOR_PAIR(2));
			wrefresh(*confirm_wnd);
			wrefresh(*cancel_wnd);
		}
		else if (127 > symbol && 32 <= symbol)
		{
			wprintw(*text_subwnd, "%c", symbol);
			fname[index] = symbol;
			index++;
		}
	}
	if (index == 0)
	{
		strncpy(fname, "text.txt", 9);
	}
	else
	{
		fname[index] = '\0';
	}

	if (ret == 0)
	{
		*file = fopen(fname, "a+");
	}

	wclear(*wnd);
	wrefresh(*wnd);
	delwin(*wnd);

	curs_set(1);

	return ret;
}

/* Wrapper to create window, asking user to save file or not */
int _fsave_window(WINDOW **pwnd, WINDOW **wnd, WINDOW **subwnd,
				WINDOW **confirm_wnd, WINDOW **confirm_subwnd,
				WINDOW **cancel_wnd, WINDOW **cancel_subwnd)
{
	/*
	* Declare:
	* - symbol - variable, storing return value of wgetch() function;
	* - ret - return value of function, also used to process confirm and cancel
	* 	buttons.
	*/
	int symbol;
	int ret = 0;

	curs_set(0);

	/* Call _create_window wrapper to create windows and fill them with info */
	_create_window(pwnd, wnd, subwnd, 10, 50, 5, 40, '|', '-');
	wmove(*subwnd, 0, (50/2 - strlen("Save changes in the file?")/2));
	wprintw(*subwnd, "Save changes in the file?");
	wbkgd(*wnd, COLOR_PAIR(2));
	_create_window(subwnd, confirm_wnd, confirm_subwnd, 3, 20, 5, 1, ' ', ' ');
	_create_window(subwnd, cancel_wnd, cancel_subwnd, 3, 20, 5, 27, ' ', ' ');
	wmove(*confirm_subwnd, 0, (20/2 - strlen("Confirm")/2 - 1));
	wprintw(*confirm_subwnd, "Confirm");
	wmove(*cancel_subwnd, 0, (20/2 - strlen("Cancel")/2 - 1));
	wprintw(*cancel_subwnd, "Cancel");

	/*
	* Set background color to usual color palette to every window, except
	* for the confirm window, mark it as selected option.
	*/
	wbkgd(*wnd, COLOR_PAIR(1));
	wbkgd(*subwnd, COLOR_PAIR(1));
	wbkgd(*confirm_wnd, COLOR_PAIR(2));
	wbkgd(*cancel_wnd, COLOR_PAIR(1));
	
	wrefresh(*wnd);
	
	/* Read user input to process arrowkey left and right, and Enter */
	keypad(*subwnd, true);
	while(1)
	{
		symbol = wgetch(*subwnd);
		if ('\n' == symbol)
		{
			break;
		}
		else if (symbol == 260)
		{
			ret = 0;
			wbkgd(*confirm_wnd, COLOR_PAIR(2));
			wbkgd(*cancel_wnd, COLOR_PAIR(1));
			wrefresh(*confirm_wnd);
			wrefresh(*cancel_wnd);
		}
		else if (symbol == 261)
		{
			ret = -1;
			wbkgd(*confirm_wnd, COLOR_PAIR(1));
			wbkgd(*cancel_wnd, COLOR_PAIR(2));
			wrefresh(*confirm_wnd);
			wrefresh(*cancel_wnd);
		}
	}

	wclear(*wnd);
	wrefresh(*wnd);
	delwin(*wnd);

	curs_set(1);

	return ret;
}

int second_task()
{
	puts("Second task");

	/*
	* Declare:
	* - file - file pointer;
	* - 'buf' and 'tmp_buf' - char arrays, 'buf' is used to store data, read
	*	from file and 'tmp_buf' - to store data to write to a file;
	* - fname - char array to store filename;
	* - symbol - variable, storing last char of user input, read from wgetch();
	* - ret - variable, storing number of bytes read;
	* - index - variable, storing current index where char is written into
	* 	tmp_buf.
	* All WINDOW * variables is used for creating terminal windows, if variable
	* ends with '_wnd' - this window is storing window itself and window border,
	* if ends with '_subwnd' - window is created with derwin of a '_wnd' and
	* locates right inside the window border.
	*/
	FILE * file = NULL;
	char buf[4096], tmp_buf[4096];
	char fname[45];
	int symbol;
	size_t ret = 0;
	size_t index = 0;

	WINDOW *left_wnd;
	WINDOW *left_subwnd;

	WINDOW *top_left_wnd;
	WINDOW *top_left_subwnd;

	WINDOW *top_right_wnd;
	WINDOW *top_right_subwnd;

	WINDOW *main_wnd;
	WINDOW *main_subwnd;

	WINDOW *fopen_wnd;
	WINDOW *fopen_subwnd;
	WINDOW *fopen_name_wnd;
	WINDOW *fopen_name_subwnd;

	WINDOW *fsave_wnd;
	WINDOW *fsave_subwnd;

	WINDOW *confirm_wnd;
	WINDOW *confirm_subwnd;
	WINDOW *cancel_wnd;
	WINDOW *cancel_subwnd;

	initscr(); /* Init ncurses lib */
	signal(SIGWINCH, sig_winch); /* Set signal handler for resizing terminal window */
	
	ioctl(fileno(stdout), TIOCGWINSZ, (char *) &size); /* Get current size of a terminal */
	
	cbreak(); /* */
	
	noecho(); /* Disable write of user input on a screen */

	curs_set(1); /* Set cursor invisible */
	start_color(); /* */
	init_pair(1, COLOR_WHITE, COLOR_BLACK); /* Init color pair for a usual color palette */
	init_pair(2, COLOR_BLACK, COLOR_WHITE); /* Init color pair for a 'selected' color palette */
	bkgd(COLOR_PAIR(1)); /* Set background color to usual color palette */
	refresh(); /* Update stdscr */

	/* Create 4 standart windows, using _new_window() wrapper */
	_new_window(&top_left_wnd, &top_left_subwnd, 3, (size.ws_col/2)-3, 0, 4, '|', '-');
	_new_window(&top_right_wnd, &top_right_subwnd, 3, (size.ws_col/2), 0, (size.ws_col/2), '|', '-');
	_new_window(&left_wnd, &left_subwnd, (size.ws_row-2), 5, 2, 0, '|', '-');
	_new_window(&main_wnd, &main_subwnd, (size.ws_row-2), (size.ws_col-4), 2, 4, '|', '-');

	/* Set background of every window to usual color palette */
	wbkgd(top_left_wnd, COLOR_PAIR(1));
	wbkgd(top_right_wnd, COLOR_PAIR(1));
	wbkgd(left_wnd, COLOR_PAIR(1));
	wbkgd(main_wnd, COLOR_PAIR(1));
	wbkgd(top_left_subwnd, COLOR_PAIR(1));
	wbkgd(top_right_subwnd, COLOR_PAIR(1));
	wbkgd(left_subwnd, COLOR_PAIR(1));
	wbkgd(main_subwnd, COLOR_PAIR(1));
	
	/* Fill these windows with text */
	wattron(top_left_subwnd, A_BOLD);
	wprintw(top_left_subwnd, "No file opened");
	wattroff(top_left_subwnd, A_BOLD);
	

	wprintw(top_right_subwnd, "  \"F1\" - to open or create a file,  \"F2\" - to save a file,  \"F3\" - exit application");
	wrefresh(top_right_wnd);

	/* Fill left window with lines layout */
	wattron(left_subwnd, A_BOLD);
	for (uint16_t i = 0; i < (size.ws_row-4); i++)
	{
		wprintw(left_subwnd, "%d\n", i);
	}
	wattroff(left_subwnd, A_BOLD);

	wprintw(main_subwnd, "Simple text editor\n"
			"Use arrow keys and Enter to choose options in "
			"open and save windows\n");

	wrefresh(top_left_subwnd);
	wrefresh(top_left_wnd);
	wrefresh(left_wnd);
	wrefresh(main_wnd);

	/*
	* Main loop, processing user input, including F1, F2, F3 buttons and write
	* to a file.
	*/
	keypad(main_subwnd, true);
	while(1)
	{
		symbol = wgetch(main_subwnd);
		/*
		* If symbol == F1, then call _fopen_window wrapper, which creates
		* a window, asking user to enter file name to open.
		* If 0 is returned, means, file is opened and will be read into a
		* 'buf' var. Then all the data is written on windows, including:
		* - Number of bytes read and filename to top left window;
		* - Data read to main window.
		*/
		if (symbol == KEY_F(1))
		{
			if (_fopen_window(&main_subwnd, &fopen_wnd, &fopen_subwnd,
				&fopen_name_wnd, &fopen_name_subwnd, &confirm_wnd,
				&confirm_subwnd, &cancel_wnd, &cancel_subwnd,
				&file, fname) == 0)
			{
				ret = fread(buf, sizeof(char), 4096, file);

				wclear(top_left_subwnd);

				wattron(top_left_subwnd, A_BOLD);
				wprintw(top_left_subwnd, "Read %d bytes from \"%s\" file", ret,
						fname);
				wattroff(top_left_subwnd, A_BOLD);
				wrefresh(top_left_subwnd);

				wclear(main_subwnd);
				wrefresh(main_subwnd);

				wprintw(main_subwnd, "%s", buf);
				wrefresh(main_wnd);
			}
		}
		/* 
		* If symbol == F2 and file is opened, then call _fsave_window wrapper,
		* which creates a window, asking user to choose whether to save file or
		* not.
		* If 0 is returned, means, user chose to save the file.
		*/
		else if (symbol == KEY_F(2))
		{
			if (file != NULL && _fsave_window(&main_subwnd,
				&fopen_wnd, &fopen_subwnd,
				&confirm_wnd,	&confirm_subwnd,
				&cancel_wnd, &cancel_subwnd) == 0)
				{
					fwrite(tmp_buf, (index), 1, file);
					break;
				}
		}
		/* If symbol == F3, then exit binary without saving */
		else if (symbol == KEY_F(3))
		{
			break;
		}
		/* Print user input into a tmp_buf */
		else if ((127 > symbol && 32 <= symbol) || 10 == symbol)
		{
			if (file != NULL)
			{
				wprintw(main_subwnd, "%c", symbol);
				tmp_buf[index] = symbol;
				index++;
			}
		}
	}

	if (file != NULL)
		fclose(file);

	endwin();
	exit(EXIT_SUCCESS);
}