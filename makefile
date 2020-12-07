CC = gcc
LDFLAGS = -lprocps
TASKMANAGER = taskmanager
UI = ui
KEYLOG = keylog
TRASH = trash
ELF = elf

all: $(TASKMANAGER) $(UI) $(KEYLOG) smth.c smth.h $(TASKMANAGER) $(UI) $(KEYLOG) $(TRASH) $(ELF)
	
$(TASKMANAGER): smth.o $(TASKMANAGER).o
	$(CC) $^ -o $@ $(LDFLAGS)

$(UI): smth.o $(UI).o
	$(CC) $^ -o $@ $(LDFLAGS)

$(KEYLOG): $(KEYLOG).c
	$(CC) $^ -o $@

$(TRASH): $(TRASH).c
	$(CC) $^ -o $@

$(ELF): $(ELF).c
	$(CC) $^ -o $@

clean:
	if test -f $(TASKMANAGER); then rm $(TASKMANAGER); fi
	if test -f $(UI); then rm $(UI); fi
	if test -f $(KEYLOG); then rm $(KEYLOG); fi
	if test -f $(TRASH); then rm $(TRASH); fi
	if test -f $(ELF); then rm $(ELF); fi