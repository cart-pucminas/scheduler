SHELL=/bin/sh
CLASS=CLASS_B
SFILE=config/suite.def

BINDIR = $(CURDIR)/bin

CC = gcc
CFLAGS = -fopenmp --std=c99 -O3 -DCLASS=$(CLASS)
LIBS = -lm ../../../libsrc/libgomp/libgomp/build/.libs/libgomp.a

all: is-dynamic is-srr

is-dynamic:
	$(CC) is.c $(CFLAGS) -DSCHEDULE=SCHEDULE_DYNAMIC -o $(BINDIR)/is.$(CLASS).dynamic $(LIBS)

is-srr:
	$(CC) is.c $(CFLAGS) -DSCHEDULE=SCHEDULE_SRR -o $(BINDIR)/is.$(CLASS).srr $(LIBS)
	
clean:
	@rm -rf $(BINDIR)/*



