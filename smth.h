#pragma once

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

static const unsigned int screensize = 20;

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

void init_array(Array *, size_t );
void insert_array(Array *, Data );
void free_array(Array *);

int cmp_1(Data *, Data *);
int cmp_2(Data *, Data *);
int cmp_3(Data *, Data *);
int cmp_4(Data *, Data *);
int cmp_5(Data *, Data *);
int cmp_6(Data *, Data *);
int cmp_7(Data *, Data *);

int getscreensize(int * , int * );
int gotoXY(int , int );
int setbgcolor(int );
int setfgcolor(int );

void ui(Array *, unsigned int , unsigned int , char );
