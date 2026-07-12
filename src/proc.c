/*  This file is part of SysMonitor.
 * SysMonitor is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3 of the License.
 * SysMonitor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with SysMonitor. If not, see <https://www.gnu.org/licenses/>. 
 */

#include "../include/proc.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void remove_process(proc_data *array, size_t *count, size_t index)
{
	if (array[index].str != NULL)
		free(array[index].str);
	for (size_t i = index; i + 1 < *count; i++)
	{
		array[i] = array[i + 1];
	}
	(*count)--;
}

double get_cpu_usage(stat_data *st_data)
{
	// text line
	char s[200];
	// reading values
	unsigned long long int user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
	int usage;

	FILE *f = fopen("/proc/stat", "r");
	// the tri-fecta
	fseek(f, 0L, SEEK_SET);
	fgets(s, 200, f);
	if (sscanf(s, "cpu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu", &user, &nice, &system, &idle,
		&iowait, &irq, &softirq, &steal, &guest, &guest_nice) != 11) // always true for some reason
	{
		// TODO: Error Check sscanf
	}


	/*
	 * Calculating formula from
	 * https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
	 * And as in the the htop source code
	 */
	unsigned long long int prevIdle = idle + iowait;
	unsigned long long int prevNonIdle = user + nice + system + irq + softirq + steal;
	unsigned long long int prevTotal = prevIdle + prevNonIdle;

	usleep(500000);

	// do it again
	freopen("/proc/stat", "r", f);

	fseek(f, 0L, SEEK_SET);
	fgets(s, 200, f);
	if (sscanf(s, "cpu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",  &user, &nice, &system, &idle,
		&iowait, &irq, &softirq, &steal, &guest, &guest_nice) != 11)
	{
		// TODO: Error check sscanf
	}
	
	unsigned long long int idleTotal = idle + iowait;
	unsigned long long int nonIdle = user + nice + system + irq + softirq + steal;
	unsigned long long int total = idleTotal + nonIdle;

	/*
	 * See:
	 * https://github.com/htop-dev/htop/blob/15652e7b8102e86b3405254405d8ee5d2a239004/linux/LinuxProcessList.c
	 * Line 1299
	 */
	#define WRAP_SUBTRACT(a,b) (((a) > (b)) ? (a) - (b) : 0)
	unsigned long long int idled = WRAP_SUBTRACT(idleTotal, prevIdle);
	unsigned long long int totald = WRAP_SUBTRACT(total, prevTotal);
	#undef WRAP_SUBTRACT

	st_data->cpu_total_d = totald;

	if (totald == 0) { fprintf(stderr, "Division BY ZERO::totald is zero");}
	else usage = (double)(totald - idled) / totald * 100.0;

	fclose(f);
	return usage;
}

double get_mem_usage(stat_data* st_data)
{
	unsigned long long int mem_total = 0;
	unsigned long long int mem_free = 0;
	unsigned long long int mem_avail = 0;
	char s1[100]; // meminfo seperated in lines in /proc/meminfo
	char s2[100];
	char s3[100];
	double usage = 0;

	FILE* f = fopen("/proc/meminfo", "r");
	fgets(s1, 100, f);
	fgets(s2, 100, f);
	fgets(s3, 100, f);

	sscanf(s1, "MemTotal: %llu" , &mem_total);
	sscanf(s2, "MemFree: %llu", &mem_free);
	sscanf(s3, "MemAvailable: %llu", &mem_avail);

	unsigned long long int mem_used = mem_total - mem_avail;
	st_data->mem_total = mem_total;
	usage = ((double)mem_used / mem_total) * 100.0;
	fclose(f);
	return usage;
}

double get_swap_usage()
{
	double usage = 0;
	char s[200];

	FILE* f = fopen("/proc/swaps", "r");
	fgets(s, 200, f);
	fgets(s, 200, f); // second line has the values

	unsigned long long int used = 0;
	unsigned long long int size = 0;
	sscanf(s, "%*s %*s %llu %llu",  &size, &used);
	usage = (double)used / size * 100.0;

	fclose(f);
	return usage;
}


// Returns cpu_time
unsigned long long int get_proc_data(long pid, unsigned long long int* rss)
{
	char str[64];
	snprintf(str, sizeof(str), "/proc/%ld/stat", pid);

	FILE* st_file = fopen(str, "r");
	if (st_file == NULL)
	{
		if (errno == ENOENT || errno == ESRCH) // failed to open file, because it exited
		{
			return 0;
		}
    		perror(str);
		return 0;
	}

	char bf[4026];
	fgets(bf, sizeof(bf), st_file);

	char *p = strchr(bf, ')');
	if (!p)
		return 0;
	p += 2;

	unsigned long long int utime = 0, stime = 0;
	int field = 3;
	char *save;
	char *tok = strtok_r(p, " ", &save);
	while (tok)
	{
		if (field == 14)
			utime = strtoull(tok, NULL, 10);

		if (field == 15)
		{
			stime = strtoull(tok, NULL, 10);
		}
		if (field == 24)
		{
			*rss = strtoull(tok, NULL, 10);
		}

		tok = strtok_r(NULL, " ", &save);
		field++;
	}

	fclose(st_file);

	return utime + stime;
}

void get_name_proc(long pid, char** str)
{
	char buf[64];
	snprintf(buf, sizeof(buf), "/proc/%ld/stat", pid);
	FILE* st_file = fopen(buf, "r");
	if (st_file == NULL)
	{
		if (errno == ENOENT || errno == ESRCH) // failed to open file, because it exited
		{
			return;
		}
    		perror(buf);
		return;
	}


	char bf[526];
	fgets(bf, sizeof(bf), st_file);
	char *start = strrchr(bf, '(');
	char *end = strchr(bf, ')');
	size_t len = end - start - 1;

	if (*str != NULL)
		free(*str);
	*str = malloc(sizeof(char) * len + 1);
	memcpy(*str, start + 1, len);
	(*str)[len] = '\0';

	fclose(st_file);
}

void update_proc_array(DIR* dir, proc_data** pid_array, size_t* pid_array_size, size_t* pid_count)
{
	// processes
	rewinddir(dir);
	struct dirent *p_dir;
	while ((p_dir = readdir(dir)) != NULL)
	{
		if (p_dir->d_type == DT_DIR)
		{
			char *end;
			errno = 0;
			long pid = strtol(p_dir->d_name, &end, 10);
			
			if (*end != '\0' || pid <= 0)
			{
				continue;
			}
			if (errno)
			{
				perror("ERROR: strtol");
				continue;
			}

			int ind = find_process(*pid_array, *pid_count, pid);
			if (ind != -1)
			{
				// exists
				(*pid_array)[ind].seen = 1;
			}
			else
			{
				// doesnt exist
				(*pid_array)[*pid_count].pid = pid;
				(*pid_array)[*pid_count].seen = 1;
				(*pid_array)[*pid_count].proc_total = 0;
				(*pid_array)[*pid_count].str = NULL;
				(*pid_count)++;
			}

			if (*pid_count >= *pid_array_size)
			{
				size_t tmp_size = (*pid_array_size == 0) ? 16 : *pid_array_size * 2;
				proc_data* tmp = realloc(*pid_array, tmp_size * sizeof(*tmp));
				if (tmp == NULL)
				{
					fprintf(stderr, "ERROR::PID_ARRAY_RESIZE_MALLOC_FAILED");
					return; // cant quite do anything from here
				}
				*pid_array = tmp;
				*pid_array_size = tmp_size;
				// printf("RESIZED!!!!");
			}
		}
	}

	// Set all seen vars
	for (int i = 0; i < *pid_count; i++)
	{
		// already not seen
		if ((*pid_array)[i].seen == 0)
		{
			// printf("Removed Process: %ld", (*pid_array)[i].pid); 
			remove_process(*pid_array, pid_count, i);
			continue;
		}
		(*pid_array)[i].seen = 0;
	}
}

int find_process(proc_data *array, size_t count, long pid)
{
	for (size_t i = 0; i < count; i++)
	{
		if (array[i].pid == pid)
			return i;
	}

	return -1;
}

double get_usage_for_proc(unsigned long long int new_time, unsigned long long int prev_time, stat_data *st_data)
{
	// compute with prev time 
	unsigned long long int proc_delta = new_time - prev_time;

	return 100.0 * proc_delta * sysconf(_SC_NPROCESSORS_ONLN) / st_data->cpu_total_d;
}
