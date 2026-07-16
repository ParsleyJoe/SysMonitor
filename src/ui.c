/*  This file is part of SysMonitor.
 * SysMonitor is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3 of the License.
 * SysMonitor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with SysMonitor. If not, see <https://www.gnu.org/licenses/>. 
 */

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

void draw_controls(int y)
{
	int x = getmaxx(stdscr) - strlen("Hor.arrows: SortChange"); // TODO: Add the longest string here

	attron(COLOR_PAIR(1));
	mvprintw(y, x, "Controls");
	mvprintw(++y, x, "Q: quit");
	mvprintw(++y, x, "F9: Kill");
	mvprintw(++y, x, "Vert.arrows: Scroll");
	mvprintw(++y, x, "Hor.arrows: SortChange");
	attroff(COLOR_PAIR(1));
}
