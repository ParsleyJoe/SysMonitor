#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include <ncurses.h>
#include <pthread.h>

#include "../include/proc.h"
#include "../include/ui.h"

// global.? idk
static volatile sig_atomic_t running = 1; 
proc_data* shared_pid_array = NULL;
int shared_pid_count = 0;
pthread_mutex_t shared_data_mutex = PTHREAD_MUTEX_INITIALIZER;
double sh_cpu_usage;
	
void sigint_func(int sig)
{
	(void)sig; // silences warning
	running = 0;
}

void *worker_thread(void* args)
{
	// Init data
	stat_data st_data;
	signal(SIGINT, sigint_func);

	DIR* dir = opendir("/proc");

	proc_data *pid_array = malloc(sizeof(proc_data) * 30);
	size_t pid_array_size = 30;
	size_t pid_count = 0; // valid processes
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
				pid_array[p].usage = MIN(us, 100.0);
				get_name_proc(pid_array[p].pid, &pid_array[p].str);
			}
			p++;
		}

		pthread_mutex_lock(&shared_data_mutex);
			if (shared_pid_array != NULL)
			{
				for (int i = 0; i < shared_pid_count; i++)
				{
					free(shared_pid_array[i].str);
				}
				free(shared_pid_array);
			}

			shared_pid_array = malloc(sizeof(proc_data) * pid_count);
			shared_pid_count = pid_count;
			memcpy(shared_pid_array,
			       pid_array,
			       pid_count * sizeof(proc_data));
			for (int i = 0; i < pid_count; i++)
			{
				if (pid_array[i].str != NULL)
					shared_pid_array[i].str = strdup(pid_array[i].str);
				else
					shared_pid_array[i].str = NULL;
			}
		pthread_mutex_unlock(&shared_data_mutex);
	}

	closedir(dir);

	return NULL;
}

static int cmp_proc(const void* arg1, const void* arg2)
{    
	// if (arg1 < arg2) return -1;
	//    	if (arg1 > arg2) return 1;
	// return 0;
	
	proc_data* p1 = (proc_data*)arg1;
	proc_data* p2 = (proc_data*)arg2;

        if (p1->usage < p2->usage) return 1;
        if (p1->usage > p2->usage) return -1;
        return 0;
}

int main(void)
{

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

	pthread_t worker;
	pthread_create(&worker, NULL, worker_thread, NULL);
	int top_proc = 0; // top visible process

	// loop and print delta
	while(running)
	{
		erase();

		// Drawing
		draw_title();
		draw_usages(sh_cpu_usage, 0.0, 0.0);

		int rows, cols;
		getmaxyx(stdscr, rows, cols);
		int start_y = getmaxy(stdscr) * 0.2;
		int visible = rows - 2;

		pthread_mutex_lock(&shared_data_mutex);

		if (shared_pid_array != NULL)
			qsort(shared_pid_array, shared_pid_count, sizeof(proc_data), cmp_proc);

		for (int i = 0; i < visible; i++)
		{
			int index = top_proc + i;
			if (index >= shared_pid_count)
				break;

			proc_data *curr = &(shared_pid_array[index]);
			mvprintw(start_y + (i + 1), 0,
				"%ld %s %lf", curr->pid, curr->str, curr->usage);
		}
		pthread_mutex_unlock(&shared_data_mutex);

		refresh();

		int ch = getch();
		switch (ch) 
		{
		case 'q':
		case 'Q':
			running = false;
			break;
		case KEY_UP:
			top_proc--;
			if (top_proc < 0)
				top_proc = 0;
			break;
		case KEY_DOWN:
			top_proc++;
			if (top_proc > shared_pid_count)
				top_proc = shared_pid_count;
			break;
		}
		napms(30);
	}

	attroff(COLOR_PAIR(1));

	endwin();
	free(shared_pid_array);
	printf("bye\n");
	return 0;
}
