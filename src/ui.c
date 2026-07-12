#include "../include/ui.h"
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

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

	mvprintw(y, 0, "CPU: ");
	draw_usage(cpu_usage, y, strlen("CPU: ") + 1);
	++y;
	mvprintw(y, 0, "MEM: ");
	draw_usage(mem_usage, y, strlen("MEM: ") + 1);
	++y;
	mvprintw(y, 0, "SWAP: ");
	draw_usage(swap_usage, y, strlen("SWAP: "));
}

void draw_usage(double usage, int y, int x)
{
	int colored_bars = (usage + 9) / 10.0;
	int us_x = 11 + x;

	char *colored = NULL;
	if (colored_bars > 0)
	{
		colored = malloc(sizeof(char) * (10 + 1));
		if (colored == NULL)
		{
			fprintf(stderr, "ERROR::draw_usage()::colored::MALLOC FAILED\n");
			return;
		}
		memset(colored, ' ', 10);
		memset(colored, '|', colored_bars);
		colored[colored_bars] = '\0';
	}

	if (colored != NULL)
		mvprintw(y, x, "%s", colored);
	mvprintw(y, us_x, "%.1lf%%", usage);
	free(colored);
}
