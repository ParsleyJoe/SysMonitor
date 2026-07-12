/* This file is part of SysMonitor.
 * SysMonitor is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3 of the License.
 * SysMonitor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with SysMonitor. If not, see <https://www.gnu.org/licenses/>. 
 */

#ifndef PROC
#define PROC
#include <signal.h>
#include <dirent.h>


typedef struct stat_data {
	unsigned long long int cpu_total_d;
	unsigned long long int mem_total;
} stat_data;

typedef struct proc_data {
	long pid;
	int seen;
	unsigned long long int proc_total;
	double cpu_usage;
	double mem_usage;
	char* str;
} proc_data;

int find_process(proc_data *array, size_t count, long pid);
void remove_process(proc_data *array, size_t *count, size_t index);
double get_cpu_usage(stat_data *st_data);
double get_mem_usage(stat_data* st_data);
double get_swap_usage();
unsigned long long int get_proc_data(long pid, unsigned long long int* rss);
void update_proc_array(DIR* dir, proc_data** pid_array, size_t* pid_array_size, size_t* pid_count);
int find_process(proc_data *array, size_t count, long pid);
double get_usage_for_proc(unsigned long long int new_time, unsigned long long int prev_time, stat_data *st_data);
void get_name_proc(long pid, char** str);
#endif
