#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>

// global.? idk
volatile sig_atomic_t running = 1; 

typedef struct stat_data{
	unsigned long long int cpu_total_d;
} stat_data;

double get_cpu_usage(stat_data *st_data)
{
	// text line
	char s[200];
	// reading values
	unsigned long long int user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
	int usage;

	FILE *f = fopen("/proc/stat", "r");
	freopen("/proc/stat", "r", f);

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
	 * and the htop code
	 */
	unsigned long long int prevIdle = idle + iowait;
	unsigned long long int prevNonIdle = user + nice + system + irq + softirq + steal;
	unsigned long long int prevTotal = prevIdle + prevNonIdle;

	sleep(1);

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

	if (totald == 0) { printf("Division BY ZERO");}
	else usage = (double)(totald - idled) / totald * 100.0;

	return usage;
}

double get_mem_usage()
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
	usage = ((double)mem_used / mem_total) * 100.0;
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
	return usage;
}

double get_usage_for_proccess(stat_data *st_data, long pid)
{
	double usage;
	struct dirent* dirnt;
	unsigned long long int procd = 0;
	char str[64];
	snprintf(str, sizeof(str), "/proc/%ld/stat", pid);
	printf("%s\n", str);

	FILE* st_file = fopen(str, "r");
	if (st_file == NULL)
	{
		printf("ERROR: Failed to open file %s\n", str);
		return -1.0;
	}
	return 100 * (st_data->cpu_total_d - procd);
}

void sigint_func(int sig)
{
	(void)sig; // silences warning
	running = 0;
}



int main(void)
{

	stat_data st_data;
	signal(SIGINT, sigint_func);

	DIR* dir = opendir("/proc");

	// loop and print delta
	while(running)
	{
		double cpu_usage = 0;
		double mem_usage = 0;
		double swap_usage = 0;

		cpu_usage = get_cpu_usage(&st_data); // sleeps for 1
		mem_usage = get_mem_usage();
		swap_usage = get_swap_usage();

		// processes
		struct dirent *p_dir;
		while ((p_dir = readdir(dir)) != NULL)
		{
			if (p_dir->d_type == DT_DIR)
			{
				char *end;
				long pid = strtol(p_dir->d_name, &end, 10);
				
				if (*end != '\0')
				{
					printf("ERROR: Sus process found %s\n", p_dir->d_name);
					continue;
				}
				if (errno)
				{
					perror("ERROR: strtol");
					continue;
				}

				get_usage_for_proccess(&st_data, pid);
			}
		}

		printf("Usage: %f\n", cpu_usage);
		printf("Mem Usage: %f\n", mem_usage);
		printf("Swap Usage: %f\n", swap_usage);
		fflush(stdout);
	}

	printf("bye\n");
	return 0;
}
