CC = gcc
CFLAGS = -w
LDFLAGS = -lprocps
DAEMON = daemon
UI = ui
INCLUDE = smth

all: smth.c smth.h daemon.c ui.c 
	$(CC) $(CFLAGS) -c $(INCLUDE).c $(LDFLAGS)
	$(CC) $(CFLAGS) -c $(DAEMON).c $(LDFLAGS)
	$(CC) $(CFLAGS) -c $(UI).c $(LDFLAGS)
	$(CC) $(CFLAGS) -o $(DAEMON) $(DAEMON).o $(INCLUDE).o $(LDFLAGS)
	$(CC) $(CFLAGS) -o $(UI) $(UI).o $(INCLUDE).o $(LDFLAGS)

clean:
	if test -f *.o; then rm *.o; fi
	if test -f $(DAEMON); then rm $(DAEMON); fi
	if test -f $(UI); then rm $(UI); fi