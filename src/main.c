#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include <ncurses.h>

#include "../include/proc.h"
#include "../include/ui.h"

// global.? idk
static volatile sig_atomic_t running = 1; 

void sigint_func(int sig)
{
	(void)sig; // silences warning
	running = 0;
}

int main(void)
{

	// Init data
	stat_data st_data;
	signal(SIGINT, sigint_func);

	DIR* dir = opendir("/proc");

	proc_data *pid_array = malloc(sizeof(proc_data) * 30);
	size_t pid_array_size = 30;
	size_t pid_count = 0; // valid processes
	

	// Init ncurses
	initscr();
	noecho();
	cbreak();
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);
	curs_set(1);

	if (has_colors() == FALSE)
	{
		endwin();
		fprintf(stderr, "Terminal doesn't support colors.\n");
		return 1;
	}

	start_color();
	init_pair(1, COLOR_BLACK, COLOR_BLUE); // fore & background colors.

	attron(COLOR_PAIR(1));

	// loop and print delta
	while(running)
	{
		double cpu_usage = 0;
		double mem_usage = 0;
		double swap_usage = 0;


		// changes the dynamic array elements, without modifying the elements themselves
		update_proc_array(dir, &pid_array, &pid_array_size, &pid_count);

		cpu_usage = get_cpu_usage(&st_data); // sleeps for 1
		mem_usage = get_mem_usage();
		swap_usage = get_swap_usage();

		int p = 0;
		while(p < pid_count)
		{
			if (pid_array[p].pid != 0)
			{
				unsigned long long int new_time = get_time_for_proc(pid_array[p].pid);
				double us = get_usage_for_proc(new_time, pid_array[p].proc_total, &st_data);
				pid_array[p].proc_total = new_time;
				//printf("%ld: %lf\n", pid_array[p].pid, us);
			}
			p++;
		}

		erase();

		// Drawing
		draw_title();
		draw_usages(cpu_usage, mem_usage, swap_usage);

		refresh();

		int ch = getch();
		switch (ch) 
		{
		case 'q':
		case 'Q':
			running = false;
			break;
		}
	}

	attroff(COLOR_PAIR(1));

	endwin();
	free(pid_array);
	closedir(dir);
	printf("bye\n");
	return 0;
}
