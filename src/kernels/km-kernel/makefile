#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Executable file.
EXEC = km

# Builds static kernel.
static:
	$(CC) *.c $(CFLAGS) -rdynamic -D_STATIC_SCHEDULE_ -o $(BINDIR)/$(EXEC).static $(LIBS)

# Builds dynamic kernel.
dynamic:
	$(CC) *.c $(CFLAGS) -rdynamic -D_DYNAMIC_SCHEDULE_ -o $(BINDIR)/$(EXEC).dynamic $(LIBS)

# Builds smart-round-robin kernel.
smart-round-robin:
	$(CC) *.c $(CFLAGS) -rdynamic -D_RUNTIME_SCHEDULE_ -o $(BINDIR)/$(EXEC).srr $(LIBS)

# Cleans compilation files.
clean:
	rm -f $(BINDIR)/$(EXEC).*
