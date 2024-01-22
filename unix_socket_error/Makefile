CC := gcc
DEPS := libadwaita-1 
CFLAGS := $(shell pkg-config --cflags $(DEPS)) -g3 -Wall
LIBS := $(shell pkg-config --libs $(DEPS))
SOURCES := $(shell find src/ -type f -name *.c)
OBJS := $(patsubst %.c, %.o, $(SOURCES))

main: main.c
	$(CC) $(CFLAGS) -o $@ main.c $(LIBS)

clean:
	find . -name "*.o" -type f -exec rm -f {} \;
	rm -rf main

.PHONY:
socket:
	socat UNIX-LISTEN:/var/run/user/1000/mysocket.sock,fork STDIO
