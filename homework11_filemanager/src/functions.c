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
	* windows - array of structure, containing:
	*  - sel_dir - index of current selected item in the window;
	*  - scanned - total amount of items in directory, contains return value of
	*    scandir();
	*  - namelist - variable, storing parameter of last scanned directory;
	*  - cwd - char array, containing current working directory;
	*  - path_wnd - WINDOW pointer for a window, storing current working
	*    directory;
	*  - main_wnd - WINDOW pointer for main window with items in the current
	*    directory.
	* - wnd - WINDOW pointer for window with size equal to terminal size;
	* - l_wnd - WINDOW pointer for left working window;
	* - r_wnd - WINDOW pointer for right working window.
	* - symbol - return value of wgetch();
	* - cur_screen - variable, indicating current working window;
	* - index & sec_index - index variables, used in 'for' loops.
	*/
	struct window_t windows[2] = {
		{1, 0, 0, 0, 0, 0},
		{1, 0, 0, 0, 0, 0}
	};
	WINDOW *wnd;
	WINDOW *l_wnd;
	WINDOW *r_wnd;
	int symbol;
	int cur_screen = 0;
	int index, sec_index;

	/* Get current working directory */
	getcwd(windows[0].cwd, CWD_SIZE);
	if (NULL == windows[0].cwd)
	{
		perror("getcwd");
	}
	strncpy(windows[1].cwd, windows[0].cwd, CWD_SIZE);

	/* Scan cwd */
	windows[0].scanned = scandir(windows[0].cwd, &windows[0].namelist, NULL, alphasort);
	windows[1].scanned = scandir(windows[1].cwd, &windows[1].namelist, NULL, alphasort);

	initscr(); /* Init ncurses lib */
	signal(SIGWINCH, sig_winch); /* Set signal handler for resizing terminal window */
	
	ioctl(fileno(stdout), TIOCGWINSZ, (char *) &size); /* Get current size of a terminal */
	
	cbreak(); /* */
	
	noecho(); /* Disable write of user input on a screen */

	curs_set(0); /* Set cursor invisible */
	start_color(); /* */
	init_pair(1, COLOR_WHITE, COLOR_BLACK); /* Init color pair for a usual color palette */
	init_pair(2, COLOR_BLACK, COLOR_WHITE); /* Init color pair for a 'selected' color palette */
	bkgd(COLOR_PAIR(1)); /* Set background color to usual color palette */
	refresh(); /* Update stdscr */

	/*
	* Create wnd, l_wnd and r_wnd, create borders, add line between main window
	* and cwd window.
	*/
	wnd = newwin(size.ws_row, size.ws_col, 0, 0);
	l_wnd = derwin(wnd, size.ws_row, (size.ws_col/2), 0, 0);
	box(l_wnd, '|', '-');
	wmove(l_wnd, 2, 1);
	whline(l_wnd, '-', (size.ws_col/2)-2);
	windows[0].main_wnd = derwin(l_wnd, (size.ws_row-4), (size.ws_col/2)-2, 3, 1);
	windows[0].path_wnd = derwin(l_wnd, 1, (size.ws_col/2)-2, 1, 1);

	r_wnd = derwin(wnd, size.ws_row, (size.ws_col/2), 0, (size.ws_col/2));
	box(r_wnd, '|', '-');
	wmove(r_wnd, 2, 1);
	whline(r_wnd, '-', (size.ws_col/2)-2);
	windows[1].main_wnd = derwin(r_wnd, (size.ws_row-4), (size.ws_col/2)-2, 3, 1);
	windows[1].path_wnd = derwin(r_wnd, 1, (size.ws_col/2)-2, 1, 1);

	/* Write cwd char array to path windows */
	wattron(windows[0].path_wnd, COLOR_PAIR(2));
	wprintw(windows[0].path_wnd, "%s", windows[0].cwd);
	wattroff(windows[0].path_wnd, COLOR_PAIR(2));

	wprintw(windows[1].path_wnd, "%s", windows[1].cwd);

	/* Fill both main windows */
	for (index = 0; index < 2; ++index)
	{
		for (sec_index = 1; sec_index < windows[index].scanned; ++sec_index)
		{
			if (sec_index == windows[index].sel_dir && index == cur_screen) 
			{
				wattron(windows[index].main_wnd, COLOR_PAIR(2));
			}

			wprintw(windows[index].main_wnd, "%s\n", windows[index].namelist[sec_index]->d_name);

			if (sec_index == windows[index].sel_dir && index == cur_screen)
				wattroff(windows[index].main_wnd, COLOR_PAIR(2));
		}
	}

	wrefresh(wnd);
	
	/* Main loop, processing keyboard */
	keypad(windows[0].main_wnd, true);
	keypad(windows[1].main_wnd, true);
	while(1)
	{
		symbol = wgetch(windows[cur_screen].main_wnd);

		/*
		* If TAB is pressed -> change current working screen, colorize both
		* windows to indicate which one is the current window, we're working on.
		*/
		if ('\t' == symbol)
		{
			/*
			* Rewrite previous window's cwd and main windows without selection.
			*/
			wclear(windows[cur_screen].path_wnd);
			wprintw(windows[cur_screen].path_wnd, windows[cur_screen].cwd);

			wclear(windows[cur_screen].main_wnd);
			for (index = 1; index < windows[cur_screen].scanned; ++index)
			{
				wprintw(windows[cur_screen].main_wnd, "%s\n", windows[cur_screen].namelist[index]->d_name);
			}

			wrefresh(windows[cur_screen].path_wnd);
			wrefresh(windows[cur_screen].main_wnd);
			
			/* Change cur_screen to other window, either 1 or 0 */
			cur_screen = 0 == cur_screen ? 1 : 0;

			/* Rewrite current window's cwd and main windows with selection */
			wclear(windows[cur_screen].path_wnd);
			wattron(windows[cur_screen].path_wnd, COLOR_PAIR(2));
			wprintw(windows[cur_screen].path_wnd, windows[cur_screen].cwd);
			wattroff(windows[cur_screen].path_wnd, COLOR_PAIR(2));

			wclear(windows[cur_screen].main_wnd);
			for (index = 1; index < windows[cur_screen].scanned; ++index)
			{
				if (index == windows[cur_screen].sel_dir) 
				{
					wattron(windows[cur_screen].main_wnd, COLOR_PAIR(2));
				}

				wprintw(windows[cur_screen].main_wnd, "%s\n", windows[cur_screen].namelist[index]->d_name);

				if (index == windows[cur_screen].sel_dir)
					wattroff(windows[cur_screen].main_wnd, COLOR_PAIR(2));
			}

			wrefresh(windows[cur_screen].path_wnd);
		}
		/* if ARROW_KEY_UP is pressed -> navigate in the directory */
		else if (259 == symbol)
		{
			/*
			* Check if selected dir is more than 1 to stay within the limits of
			* namelist array.
			*/
			if (windows[cur_screen].sel_dir > 1)
			{
				/* Decrement selected dir */
				windows[cur_screen].sel_dir--;

				/* Rewrite main window */
				wclear(windows[cur_screen].main_wnd);
				for (index = 1; index < windows[cur_screen].scanned; ++index)
				{
					if (index == windows[cur_screen].sel_dir) 
					{
						wattron(windows[cur_screen].main_wnd, COLOR_PAIR(2));
					}

					wprintw(windows[cur_screen].main_wnd, "%s\n", windows[cur_screen].namelist[index]->d_name);

					if (index == windows[cur_screen].sel_dir)
						wattroff(windows[cur_screen].main_wnd, COLOR_PAIR(2));
				}
			}
		}
		/* if ARROW_KEY_DOWN is pressed -> navigate in the directory */
		else if (258 == symbol)
		{
			/*
			* Check if selected dir does not exceeds number of scanned items by
			* scandir().
			*/
			if (windows[cur_screen].sel_dir < windows[cur_screen].scanned-1)
			{
				/* Increment selected dir */
				windows[cur_screen].sel_dir++;

				/* Rewrite main window */
				wclear(windows[cur_screen].main_wnd);
				for (index = 1; index < windows[cur_screen].scanned; ++index)
				{
					if (index == windows[cur_screen].sel_dir) 
					{
						wattron(windows[cur_screen].main_wnd, COLOR_PAIR(2));
					}

					wprintw(windows[cur_screen].main_wnd, "%s\n", windows[cur_screen].namelist[index]->d_name);

					if (index == windows[cur_screen].sel_dir)
						wattroff(windows[cur_screen].main_wnd, COLOR_PAIR(2));
				}
			}
		}
		/*
		* if ENTER is pressed -> try to open current selected item and change
		* current working directory.
		*/
		else if ('\n' == symbol)
		{
			/* If selected item isn't a directory, then nothing will happen */
			if (windows[cur_screen].namelist[windows[cur_screen].sel_dir]->d_type != 4)
				continue;

			/* Clear both path and main windows in current window */
			wclear(windows[cur_screen].path_wnd);
			wclear(windows[cur_screen].main_wnd);

			/* Change directory, get cwd */
			chdir(windows[cur_screen].namelist[windows[cur_screen].sel_dir]->d_name);
			getcwd(windows[cur_screen].cwd, CWD_SIZE);
			if (NULL == windows[cur_screen].cwd)
			{
				perror("getcwd");
			}

			windows[cur_screen].sel_dir = 1;

			/* Free current namelist, get new one */
			for (index = 0; index < windows[cur_screen].scanned; ++index)
			{
				free(windows[cur_screen].namelist[index]);
			}

			free(windows[cur_screen].namelist);

			windows[cur_screen].scanned = scandir(windows[cur_screen].cwd, &windows[cur_screen].namelist, NULL, alphasort);

			/* Write info into path and main windows */
			wattron(windows[cur_screen].path_wnd, COLOR_PAIR(2));
			wprintw(windows[cur_screen].path_wnd, windows[cur_screen].cwd);
			wattroff(windows[cur_screen].path_wnd, COLOR_PAIR(2));

			for (index = 1; index < windows[cur_screen].scanned; ++index)
			{
				if (index == windows[cur_screen].sel_dir) 
				{
					wattron(windows[cur_screen].main_wnd, COLOR_PAIR(2));
				}

				wprintw(windows[cur_screen].main_wnd, "%s\n", windows[cur_screen].namelist[index]->d_name);

				if (index == windows[cur_screen].sel_dir)
					wattroff(windows[cur_screen].main_wnd, COLOR_PAIR(2));
			}

			wrefresh(windows[cur_screen].path_wnd);
			wrefresh(windows[cur_screen].main_wnd);
		}
		/* If F3 is pressed -> exit application */
		else if (KEY_F(3) == symbol)
		{
			break;
		}
	}

	/* Free mallocs */
	for (index = 0; index < 2; ++index)
	{
		for (sec_index = 0; sec_index < windows[index].scanned; ++sec_index)
		{
			free(windows[index].namelist[sec_index]);
		}
		free(windows[index].namelist);
	}

	endwin();
	exit(EXIT_SUCCESS);
}