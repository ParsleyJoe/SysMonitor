#ifndef PROC
#define PROC
#include <signal.h>
#include <dirent.h>


typedef struct stat_data{
	unsigned long long int cpu_total_d;
} stat_data;

typedef struct proc_data {
	long pid;
	int seen;
	unsigned long long int proc_total;
	double usage;
	char* str;
} proc_data;

int find_process(proc_data *array, size_t count, long pid);
void remove_process(proc_data *array, size_t *count, size_t index);
double get_cpu_usage(stat_data *st_data);
double get_mem_usage();
double get_swap_usage();
unsigned long long int get_time_for_proc(long pid);
void update_proc_array(DIR* dir, proc_data** pid_array, size_t* pid_array_size, size_t* pid_count);
int find_process(proc_data *array, size_t count, long pid);
double get_usage_for_proc(unsigned long long int new_time, unsigned long long int prev_time, stat_data *st_data);
void get_name_proc(long pid, char** str);
#endif
