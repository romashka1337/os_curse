#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <proc/readproc.h>
#include <proc/sysinfo.h>
#include <sys/types.h>
#include <sys/stat.h>

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
			// 0,0,0,0,0,0
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

	int d_ui, ui_d;
	char s = -1;
	unsigned int top = 0, current = 0;

	mkfifo("ui_d", 0666);
	mkfifo("d_ui", 0666);

	while (1) {
		system("clear");

		d_ui = NULL; ui_d = NULL;
		init_array(&arr, 500);

		d_ui = open("d_ui", O_RDONLY);
		unsigned int used = 2;
		char c[sizeof(used)];
		read(d_ui, (void *)&used, sizeof(used));
		// printf("%d\n", used);
		for (unsigned int it = 0; it < used; ++it) {
			Data data;
			char program[36];
			int pid;
			int ppid;
			float mem;
			long long unsigned int pgrp;
			long long unsigned int tty;
			float pcpu;
			read(d_ui, program, sizeof(program) + 1);
			read(d_ui, (void *)&pid, sizeof(pid));
			read(d_ui, (void *)&ppid, sizeof(ppid));
			read(d_ui, (void *)&mem, sizeof(mem));
			read(d_ui, (void *)&pgrp, sizeof(pgrp));
			read(d_ui, (void *)&tty, sizeof(tty));
			read(d_ui, (void *)&pcpu, sizeof(pcpu));
			strcpy(data.program, program);
			data.pid = pid;
			data.ppid = ppid;
			data.mem = mem;
			data.pgrp = pgrp;
			data.tty = tty;
			data.pcpu = pcpu;
			insert_array(&arr, data);
			// printf("%s\n", arr.data[it].program);
		}
		close(d_ui);

		
		ui(&arr, top, current, s);

		char str[6] = {0};
		read(0, str, 6);
		ui_d = open("ui_d", O_WRONLY);
		write(ui_d, str, sizeof(str) + 1);
		close(ui_d);
		if (str[0] == 'q') {
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
		if (str[0] == '1') s = 1;
		if (str[0] == '2') s = 2;
		if (str[0] == '3') s = 3;
		if (str[0] == '4') s = 4;
		if (str[0] == '5') s = 5;
		if (str[0] == '6') s = 6;
		if (str[0] == '7') s = 7;

		free_array(&arr);
	}

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

	return 0;
}