#include "smth.h"

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
		if (str[0] == 'k') { kill(arr.data[current].pid, SIGKILL); continue; }
		
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