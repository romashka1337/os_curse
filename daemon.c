#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <proc/readproc.h>
#include <proc/sysinfo.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>

const unsigned int screensize = 20;

typedef struct {
	char program[36];
	int pid;
	int ppid;
	float mem;
	long long unsigned int pgrp;
	long long unsigned int tty;
	float pcpu;
} Data;

typedef struct {
	Data *data;
	unsigned int used;
	unsigned int size;
} Array;

void init_array(Array *arr, size_t initialSize) {
	arr->data = (Data *)calloc(initialSize, sizeof(Data));

	arr->used = 0;
	arr->size = initialSize;
}

void insert_array(Array *arr, Data element) {
	strcpy(arr->data[arr->used].program, element.program);
	arr->data[arr->used].pid = element.pid;
	arr->data[arr->used].ppid = element.ppid;
	arr->data[arr->used].mem = element.mem;
	arr->data[arr->used].pgrp = element.pgrp;
	arr->data[arr->used].tty = element.tty;
	arr->data[arr->used].pcpu = element.pcpu;

	arr->used++;
}

void free_array(Array *arr) {
	free(arr->data);
	arr->data = NULL;

	arr->used = 0;
	arr->size = 0;
}

int cmp_1(Data *a, Data *b) {
	char 
		*a1 = (char *)calloc(strlen(a->program) + 1, sizeof(char)),
		*b1 = (char *)calloc(strlen(b->program) + 1, sizeof(char));
	for(unsigned int it = 0; a->program[it]; ++it) 	a1[it] = tolower(a->program[it]);
	for(unsigned int it = 0; b->program[it]; ++it) 	b1[it] = tolower(b->program[it]);
	return strcmp(a1, b1);
}
int cmp_2(Data *a, Data *b) { return a->pid > b->pid; }
int cmp_3(Data *a, Data *b) { return a->ppid > b->ppid; }
int cmp_4(Data *a, Data *b) { return a->pgrp > b->pgrp; }
int cmp_5(Data *a, Data *b) { return b->mem > a->mem; }
int cmp_6(Data *a, Data *b) { return b->tty > a->tty; }
int cmp_7(Data *a, Data *b) { return b->pcpu > a->pcpu; }

int main() {
	pid_t pid, sid;

	pid = fork();
	if (pid < 0) exit(EXIT_FAILURE);
	if (pid > 0) exit(EXIT_SUCCESS);

	umask(0);

	sid = setsid();
	if (sid < 0) exit(EXIT_FAILURE);

	if ((chdir("/home/collapsing/os")) < 0) exit(EXIT_FAILURE);

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	Array arr;

	int d_ui, ui_d;
	
	proc_t proc_info;
	PROCTAB* proc;
	char s1 = 0, s2 = 0, s3 = 0, s4 = 0, s5 = 0, s6 = 0, s7 = 0, s = -1;
	unsigned int top = 0, current = 0;

	mkfifo("ui_d", 0666);
	mkfifo("d_ui", 0666);

	while (1) {
		d_ui = NULL; ui_d = NULL;
		init_array(&arr, 500);

		proc = openproc(PROC_FILLMEM | PROC_FILLSTAT | PROC_FILLSTATUS);

		memset(&proc_info, 0, sizeof(proc_info));

		while (readproc(proc, &proc_info) != NULL) {
			Data data;
			strcpy(data.program, proc_info.cmd);
			data.pid = proc_info.tid;
			data.ppid = proc_info.ppid;
			data.mem = proc_info.size;
			data.pgrp = proc_info.pgrp;
			data.tty = proc_info.tty;

			meminfo();

			float pmem = proc_info.vm_rss * 1000ULL / kb_main_total;
			if (pmem > 999) pmem = 999;
			data.mem = pmem / 10;

			long long unsigned int 
				total_jiffies = proc_info.utime + proc_info.stime,
				seconds_since_boot = uptime(NULL, NULL);
			float
				seconds = seconds_since_boot - proc_info.start_time / Hertz,
				pcpu = (total_jiffies * 1000ULL / Hertz) / seconds;
			data.pcpu = pcpu / 10;
			insert_array(&arr, data);
		}

		char str[6] = {0};
		ui_d = open("ui_d", O_RDONLY);
		read(ui_d, str, sizeof(str) + 1);
		printf("%s\n", str);
		close(ui_d);
		// return 0;

		if (str[0] == '1') { s1 = 1; s2 = 0; s3 = 0; s4 = 0; s5 = 0; s6 = 0; s7 = 0; s = 1; }
		if (str[0] == '2') { s1 = 0; s2 = 1; s3 = 0; s4 = 0; s5 = 0; s6 = 0; s7 = 0; s = 2; }
		if (str[0] == '3') { s1 = 0; s2 = 0; s3 = 1; s4 = 0; s5 = 0; s6 = 0; s7 = 0; s = 3; }
		if (str[0] == '4') { s1 = 0; s2 = 0; s3 = 0; s4 = 1; s5 = 0; s6 = 0; s7 = 0; s = 4; }
		if (str[0] == '5') { s1 = 0; s2 = 0; s3 = 0; s4 = 0; s5 = 1; s6 = 0; s7 = 0; s = 5; }
		if (str[0] == '6') { s1 = 0; s2 = 0; s3 = 0; s4 = 0; s5 = 0; s6 = 1; s7 = 0; s = 6; }
		if (str[0] == '7') { s1 = 0; s2 = 0; s3 = 0; s4 = 0; s5 = 0; s6 = 0; s7 = 1; s = 7; }

		if (s1) qsort(arr.data, arr.used, sizeof(Data), cmp_1);
		if (s2) qsort(arr.data, arr.used, sizeof(Data), cmp_2);
		if (s3) qsort(arr.data, arr.used, sizeof(Data), cmp_3);
		if (s4) qsort(arr.data, arr.used, sizeof(Data), cmp_4);
		if (s5) qsort(arr.data, arr.used, sizeof(Data), cmp_5);
		if (s6) qsort(arr.data, arr.used, sizeof(Data), cmp_6);
		if (s7) qsort(arr.data, arr.used, sizeof(Data), cmp_7);

		if (str[0] == 'q') {
			closeproc(proc);
			free_array(&arr);
			break;
		}

		d_ui = open("d_ui", O_WRONLY);
		unsigned int used = arr.used;
		write(d_ui, (void *)&used, sizeof(used));
		for (unsigned int it = 0; it < arr.used; ++it) {
			char program[36];
			strcpy(program, arr.data[it].program);
			int pid = arr.data[it].pid;
			int ppid = arr.data[it].ppid;
			float mem = arr.data[it].mem;
			long long unsigned int pgrp = arr.data[it].pgrp;
			long long unsigned int tty = arr.data[it].tty;
			float pcpu = arr.data[it].pcpu;
			write(d_ui, program, sizeof(program) + 1);
			write(d_ui, (void *)&pid, sizeof(pid));
			write(d_ui, (void *)&ppid, sizeof(ppid));
			write(d_ui, (void *)&mem, sizeof(mem));
			write(d_ui, (void *)&pgrp, sizeof(pgrp));
			write(d_ui, (void *)&tty, sizeof(tty));
			write(d_ui, (void *)&pcpu, sizeof(pcpu));
		}
		close(d_ui);

		if (str[0] == 'u') {
			closeproc(proc);
			free_array(&arr);
			continue;
		}
		if (str[0] == '\033' && str[2] == 'A') { if (current > 0) current--; continue; }
		if (str[0] == '\033' && str[2] == 'B') { if (current < arr.used - 1) current++; continue; }
		if (str[0] == 'k') { kill(arr.data[current].pid, SIGKILL); continue; }

		closeproc(proc);
		free_array(&arr);
		sleep(1);
	}
	return 0;
}