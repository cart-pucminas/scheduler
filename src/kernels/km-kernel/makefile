#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Executable file.
EXEC = km

# Builds static kernel.
static:
	$(CC) *.c $(CFLAGS) -rdynamic -D_STATIC_SCHEDULE_ -o $(BINDIR)/$(EXEC).static $(LIBS)

# Builds static kernel.
dynamic:
	$(CC) *.c $(CFLAGS) -rdynamic -D_DYNAMIC_SCHEDULE_ -o $(BINDIR)/$(EXEC).dynamic $(LIBS)

# Cleans compilation files.
clean:
	rm -f $(BINDIR)/$(EXEC).*
