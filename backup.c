#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <proc/readproc.h>
#include <proc/sysinfo.h>
// #include <unistd.h>
// #include <signal.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <syslog.h>

const unsigned int screensize = 20;

typedef struct {
	char *program;
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
	arr->data[arr->used].program = (char *)calloc(strlen(element.program) + 1, sizeof(char));
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
	for(int it = 0; it < arr->used; ++it) {
		free(arr->data[it].program);
		arr->data[it].program = NULL;
	}

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
int cmp_6(Data *a, Data *b) { return a->tty > b->tty; }
int cmp_7(Data *a, Data *b) { return b->pcpu > a->pcpu; }

int getscreensize(int * rows, int * cols) {
	struct winsize ws;
	if (!ioctl(1, TIOCGWINSZ, &ws)) {
		(*rows) = ws.ws_row;
		(*cols) = ws.ws_col;
		return 0;
	}
		return -1;
}

int gotoXY(int x, int y) {
	int rows = 0, cols = 0;	
	getscreensize(&rows, &cols);	
	printf("\033[%d;%dH", y, x);
	return 0;
}

int setbgcolor(int COL) {
	if (COL == 0 || (COL <= 37 && COL >= 30)) {
		int c = (int)COL + 10;
		printf("\033[0;%dm", c);
		return 0;
	}
	return -1;
}

int setfgcolor(int COL) {
	if (COL == 0 || (COL <= 37 && COL >= 30)) {	
		printf("\033[0;%dm", COL);
		return 0;
	}
	return -1;
}

void ui(Array *arr, unsigned int top, unsigned int current, char s) {
	gotoXY(1, 1);
	if (s == 1) setbgcolor(30);
	printf("Program                             \t"); setbgcolor(0);
	if (s == 2) setbgcolor(30);
	printf("PID  \t"); setbgcolor(0);
	if (s == 3) setbgcolor(30);
	printf("PPID\t"); setbgcolor(0);
	if (s == 4) setbgcolor(30);
	printf("PGRP  \t"); setbgcolor(0);
	if (s == 5) setbgcolor(30);
	printf("PMEM \t"); setbgcolor(0);
	if (s == 6) setbgcolor(30);
	printf("TTY   \t"); setbgcolor(0);
	if (s == 7) setbgcolor(30);
	printf("PCPU \n"); setbgcolor(0);

	for (unsigned int it = top; it < top + screensize; ++it) {
		setfgcolor(0);
		if (it == current) setfgcolor(33);
		printf(
			"%36s\t%5d\t%4d\t%6lld\t%3.2f\t%6lld\t%3.2f\n", 
			arr->data[it].program, arr->data[it].pid, 
			arr->data[it].ppid, arr->data[it].pgrp, 
			arr->data[it].mem, arr->data[it].tty, arr->data[it].pcpu
		);
		setfgcolor(0);
	}
	printf("1..7 - sort\tup,down - move\tu - update\tk - kill\nq - quit\n");
}

int main() {
	struct termios oldt, newt;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~ICANON;
	newt.c_lflag &= ~ECHO;
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);

	Array arr;
	
	proc_t proc_info;
	PROCTAB* proc;
	char s1 = 0, s2 = 0, s3 = 0, s4 = 0, s5 = 0, s6 = 0, s7 = 0, s = -1;
	unsigned int top = 0, current = 0;
	while (1) {
		system("clear");
		proc = openproc(PROC_FILLMEM | PROC_FILLSTAT | PROC_FILLSTATUS);

		memset(&proc_info, 0, sizeof(proc_info));
		init_array(&arr, 500);

		while (readproc(proc, &proc_info) != NULL) {
			Data data;
			data.program = (char *)calloc(strlen(proc_info.cmd) + 1, sizeof(char));
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
		if (s1) qsort(arr.data, arr.used, sizeof(Data), cmp_1);
		if (s2) qsort(arr.data, arr.used, sizeof(Data), cmp_2);
		if (s3) qsort(arr.data, arr.used, sizeof(Data), cmp_3);
		if (s4) qsort(arr.data, arr.used, sizeof(Data), cmp_4);
		if (s5) qsort(arr.data, arr.used, sizeof(Data), cmp_5);
		if (s6) qsort(arr.data, arr.used, sizeof(Data), cmp_6);
		if (s7) qsort(arr.data, arr.used, sizeof(Data), cmp_7);
		
		ui(&arr, top, current, s);

		char str[6] = {0};
		read(0, str, 6);
		if (str[0] == 'u') {
			closeproc(proc);
			free_array(&arr);
			continue;
		}
		if (str[0] == 'q') {
			closeproc(proc);
			free_array(&arr);
			break;
		}
		if (str[0] == '\033' && str[2] == 'A') {
			if (current > 0) current--;
			if (current - top == 1 && top > 0) top--;
		}
		if (str[0] == '\033' && str[2] == 'B') {
			if (current < arr.used - 1) current++;
			if (current - top == screensize - 2 && top < arr.used - screensize) top++;
		}
		if (str[0] == 'k') kill(arr.data[current].pid, SIGKILL);
		if (str[0] == '1') { s1 = 1; s2 = 0; s3 = 0; s4 = 0; s5 = 0; s6 = 0; s7 = 0; s = 1; }
		if (str[0] == '2') { s1 = 0; s2 = 1; s3 = 0; s4 = 0; s5 = 0; s6 = 0; s7 = 0; s = 2; }
		if (str[0] == '3') { s1 = 0; s2 = 0; s3 = 1; s4 = 0; s5 = 0; s6 = 0; s7 = 0; s = 3; }
		if (str[0] == '4') { s1 = 0; s2 = 0; s3 = 0; s4 = 1; s5 = 0; s6 = 0; s7 = 0; s = 4; }
		if (str[0] == '5') { s1 = 0; s2 = 0; s3 = 0; s4 = 0; s5 = 1; s6 = 0; s7 = 0; s = 5; }
		if (str[0] == '6') { s1 = 0; s2 = 0; s3 = 0; s4 = 0; s5 = 0; s6 = 1; s7 = 0; s = 6; }
		if (str[0] == '7') { s1 = 0; s2 = 0; s3 = 0; s4 = 0; s5 = 0; s6 = 0; s7 = 1; s = 7; }

		closeproc(proc);
		free_array(&arr);
	}

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

	return 0;
}

// static void skeleton_daemon()
// {
// 	pid_t pid;

// 	/* Fork off the parent process */
// 	pid = fork();

// 	/* An error occurred */
// 	if (pid < 0)
// 		exit(EXIT_FAILURE);

// 	/* Success: Let the parent terminate */
// 	if (pid > 0)
// 		exit(EXIT_SUCCESS);

// 	/* On success: The child process becomes session leader */
// 	if (setsid() < 0)
// 		exit(EXIT_FAILURE);

// 	/* Catch, ignore and handle signals */
// 	//TODO: Implement a working signal handler */
// 	signal(SIGCHLD, SIG_IGN);
// 	signal(SIGHUP, SIG_IGN);

// 	 Fork off for the second time
// 	pid = fork();

// 	/* An error occurred */
// 	if (pid < 0)
// 		exit(EXIT_FAILURE);

// 	/* Success: Let the parent terminate */
// 	if (pid > 0)
// 		exit(EXIT_SUCCESS);

// 	/* Set new file permissions */
// 	umask(0);

// 	/* Change the working directory to the root directory */
// 	/* or another appropriated directory */
// 	chdir("~/os");

// 	/* Close all open file descriptors */
// 	int x;
// 	for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
// 	{
// 		close (x);
// 	}

// 	/* Open the log file */
// 	openlog ("firstdaemon", LOG_PID, LOG_DAEMON);
// }

	// skeleton_daemon();

	// while (1)
	// {
	// 	//TODO: Insert daemon code here.
	// 	syslog (LOG_NOTICE, "First daemon started.");
	// 	sleep (20);
	// 	break;
	// }

	// syslog (LOG_NOTICE, "First daemon terminated.");
	// closelog();

	// return EXIT_SUCCESS;