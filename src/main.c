/* This file is part of SysMonitor.
 * SysMonitor is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3 of the License.
 * SysMonitor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with SysMonitor. If not, see <https://www.gnu.org/licenses/>. 
 */

#include <stdio.h>
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
double sh_mem_usage;
double sh_swap_usage;
	
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
		mem_usage = get_mem_usage(&st_data);
		swap_usage = get_swap_usage();

		int p = 0;
		while(p < pid_count)
		{
			if (pid_array[p].pid != 0)
			{
				unsigned long long int mem_rss = 0;
				unsigned long long int new_time = get_proc_data(pid_array[p].pid, &mem_rss);
				double mem_usage = 100.0 * mem_rss / st_data.mem_total;
				pid_array[p].mem_usage = mem_usage;
				double us = get_usage_for_proc(new_time, pid_array[p].proc_total, &st_data);
				pid_array[p].proc_total = new_time;
				pid_array[p].cpu_usage = MIN(us, 100.0);
				if (us <= 1.0 && us != 0.0)
					fprintf(stderr, "FOUND SOME: %lf", us);

				get_name_proc(pid_array[p].pid, &pid_array[p].str);
			}
			p++;
		}

		pthread_mutex_lock(&shared_data_mutex);
			sh_cpu_usage = cpu_usage;
			sh_mem_usage = mem_usage;
			sh_swap_usage = swap_usage;
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

	for (int i = 0; i < pid_count; i++)
	{
		if (pid_array[i].str != NULL)
		{
			free(pid_array[i].str);
			pid_array[i].str = NULL;
		}
	}
	free(pid_array);


	closedir(dir);

	return NULL;
}

static int cmp_proc(const void* arg1, const void* arg2)
{
	proc_data* p1 = (proc_data*)arg1;
	proc_data* p2 = (proc_data*)arg2;

        if (p1->cpu_usage < p2->cpu_usage) return 1; // arranges in descending order, default example from cppreference, arranges in ascending
        if (p1->cpu_usage > p2->cpu_usage) return -1;
        return 0;
}

int main(void)
{

	// ------------------------
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
	init_pair(1, COLOR_BLACK, COLOR_BLUE);

	pthread_t worker;
	pthread_create(&worker, NULL, worker_thread, NULL);

	int top_proc = 0; // top visible process
	int selected_proc = 0;
	double cpu_usage = 0;
	double mem_usage = 0;
	double swap_usage = 0;

	// loop and print delta
	while(running)
	{
		erase();

		// Drawing
		draw_title();

		int rows, cols;
		getmaxyx(stdscr, rows, cols);
		int visible_procs = rows - 2;

		// ---------------------------------------------------------------------
		// Lock Mutex get data, draw process data, we don't copy the process data
		pthread_mutex_lock(&shared_data_mutex);

		cpu_usage = sh_cpu_usage;
		mem_usage = sh_mem_usage;
		swap_usage = sh_swap_usage;


		if (shared_pid_array != NULL)
			qsort(shared_pid_array, shared_pid_count, sizeof(proc_data), cmp_proc);

		int usage_x = getmaxx(stdscr) * 0.6;
		int mem_x = getmaxx(stdscr) * 0.7;
		
		int start_y = getmaxy(stdscr) * 0.2;
		attron(COLOR_PAIR(1));
		mvprintw(start_y, 0, "PID\tName");
		mvprintw(start_y, usage_x, "CPU%%<-");
		mvprintw(start_y, mem_x, "MEM%%");
		attroff(COLOR_PAIR(1));

		for (int i = 0; i < visible_procs; i++)
		{
			int index = top_proc + i;
			if (index >= shared_pid_count)
				break;

			proc_data *curr = &(shared_pid_array[index]);
			int y = start_y + (i + 1);
			if (index == selected_proc)
			{
				attron(COLOR_PAIR(1));
			}
			mvprintw(y, 0,
				"%ld\t%s\t", curr->pid, curr->str);
			move(y, usage_x);
			mvprintw(y, usage_x, "%.2lf", curr->cpu_usage);
			mvprintw(y, mem_x, "%.2lf", curr->mem_usage);

			if (index == selected_proc)
				attroff(COLOR_PAIR(1));
		}
		pthread_mutex_unlock(&shared_data_mutex);
		// Unlock Mutex
		// --------------------------------------

		draw_controls(start_y);
		draw_usages(cpu_usage, mem_usage, swap_usage);

		refresh();

		int ch = getch();
		switch (ch) 
		{
		case 'q':
		case 'Q':
			running = false;
			break;
		case KEY_UP:
			selected_proc--;
			if (selected_proc < 0)
				selected_proc = 0;
			if (selected_proc < top_proc)
				top_proc--;
			break;
		case KEY_DOWN:
			selected_proc++;
			if ((selected_proc - top_proc + start_y) > (getmaxy(stdscr) - 2))
			{
				top_proc++;
			}
			if (top_proc > shared_pid_count)
				top_proc = shared_pid_count;
			if (selected_proc >= shared_pid_count)
				selected_proc = shared_pid_count - 1;
			break;
		case KEY_F(9):
			{
			pthread_mutex_lock(&shared_data_mutex);
			kill_proc(shared_pid_array[selected_proc].pid);
			pthread_mutex_unlock(&shared_data_mutex);
			}
			break;
		}
		napms(30);
	}

	void *ret;
	pthread_join(worker, &ret);

	for (int i = 0; i < shared_pid_count; i++)
	{
		if (shared_pid_array[i].str != NULL)
			free(shared_pid_array[i].str);
	}
	free(shared_pid_array);

	endwin();
	printf("bye\n");
	return 0;
}
