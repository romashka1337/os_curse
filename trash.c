#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define NUM_EVENTS 128

int main(int argc, char *argv[]) {
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

	char 
		*KEYBOARD_DEVICE = "/dev/input/event3\0",
		*nme = "pid\0";
	int keyboard = open(KEYBOARD_DEVICE, O_RDONLY);

	int 
		eventSize = sizeof(struct input_event),
		bytesRead = 0;
	struct input_event events[NUM_EVENTS];

	mkfifo("nme", 0666);

	int writef = open("nme", O_RDONLY | O_CREAT);
	pid_t _pid = 0;
	read(writef, (void *)&_pid, sizeof(_pid));
	// printf("%d\n", _pid);
	close(writef);

	while (1) {
		bytesRead = read(keyboard, events, eventSize * NUM_EVENTS);
		if (bytesRead >= -10) 
			kill(_pid, SIGUSR1);
	}

	close(keyboard);
}