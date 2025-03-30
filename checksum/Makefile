#*******************************************************************************
#
# Sam Siewert
#
# Makefile for Unix-based serial test tool
#
# Tested on Linux  2.4.2-2 #1 Sun Apr 8 20:41:30 EDT 2001 i686 unknown
#
#*******************************************************************************

OBJECTS =

CC = gcc

INCLUDES = -I.

LDFLAGS =

LIBS = -lm

all: checksum
 
checksum: checksum.o
	$(CC) $(LDFLAGS) $(LIBS) $^ -o $@


.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean :
	rm -f *.o *.i *.s *~ \#*# core .#* .new* checksum
