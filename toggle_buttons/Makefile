CC := gcc
CFLAGS := $(shell pkg-config --cflags libadwaita-1) -g3 -Wall
LIBS := $(shell pkg-config --libs libadwaita-1)
LIBS += "-lgtk4-layer-shell"
SOURCES := $(shell find src/ -type f -name *.c)

main: main.c
	$(CC) $(CFLAGS) -o $@ $< $(OBJS) $(LIBS)
