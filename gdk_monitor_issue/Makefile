CC := gcc
CFLAGS := $(shell pkg-config --cflags gtk4) -g3 -Wall
LIBS := $(shell pkg-config --libs gtk4)
SOURCES := $(shell find src/ -type f -name *.c)

main: main.c
	$(CC) $(CFLAGS) -o $@ $< $(OBJS) $(LIBS)
