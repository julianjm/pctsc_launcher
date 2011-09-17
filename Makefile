NAME=pctsclauncher
CFLAGS=-g -Wall -o $(NAME)
GTKFLAGS=-export-dynamic `pkg-config --cflags --libs gtk+-2.0 glib-2.0`
SRCS=main.c xmlparser.c gtkmainmenu.c
CC=gcc

LBITS := $(shell getconf LONG_BIT)
 
# top-level rule to create the program.
all: main

ifeq ($(LBITS),64)
ui.gtkbuilder.o: ui.gtkbuilder.xml
		objcopy --input binary --output elf64-x86-64 --binary-architecture i386 ui.gtkbuilder.xml ui.gtkbuilder.o 
else
ui.gtkbuilder.o: ui.gtkbuilder.xml
		objcopy --input binary --output elf32-i386 --binary-architecture i386 ui.gtkbuilder.xml ui.gtkbuilder.o
endif

# compiling the source file.
main: $(SRCS) ui.gtkbuilder.o
	$(CC) $(CFLAGS) $(SRCS) ui.gtkbuilder.o $(GTKFLAGS)
 
# cleaning everything that can be automatically recreated with "make".
clean:
	/bin/rm -f $(NAME)

