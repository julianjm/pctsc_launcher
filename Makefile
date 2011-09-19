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


tcz: main
	test -d package || mkdir package
	rm -rf /tmp/pctsc_package &> /dev/null
	mkdir -p /tmp/pctsc_package/usr/local/bin
	cp $(NAME) /tmp/pctsc_package/usr/local/bin
	strip --strip-unneeded /tmp/pctsc_package/usr/local/bin/$(NAME)
	mksquashfs /tmp/pctsc_package package/$(NAME).tcz -noappend
	echo "gtk2.tcz" > package/$(NAME).tcz.dep
	( cd package ; md5sum $(NAME).tcz ) > package/$(NAME).tcz.md5.txt
	( cd /tmp/pctsc_package ; find ) > package/$(NAME).tcz.list
	
	
	
