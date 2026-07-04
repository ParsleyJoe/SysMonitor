#include "../include/ui.h"
#include <ncurses.h>
#include <string.h>

void draw_title()
{
	const char* title = "System Monitor";
	int titleLen = strlen(title);
	int x = (COLS - strlen(title)) / 2;

	mvhline(0, 0, '=', COLS);
	mvprintw(0, x, "%s", title);
}

// Draw CPU, MEM, SWAP usages
void draw_usages(double cpu_usage, double mem_usage, double swap_usage)
{
	int y = getmaxy(stdscr);
	y = y * 0.08;

	mvprintw(y, 0, "CPU: %lf\nMEM: %lf\nSWAP: %lf", cpu_usage, mem_usage, swap_usage);
}
