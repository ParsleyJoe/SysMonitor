/*  This file is part of SysMonitor.
 * SysMonitor is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3 of the License.
 * SysMonitor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with SysMonitor. If not, see <https://www.gnu.org/licenses/>. 
 */

#ifndef UI
#define UI

void draw_title();
void draw_usages(double cpu_usage, double mem_usage, double swap_usage);
void draw_usage(double usage, int y, int x);

#endif
