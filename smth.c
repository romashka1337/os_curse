#include "smth.h"

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
