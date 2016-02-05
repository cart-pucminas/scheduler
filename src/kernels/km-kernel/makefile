#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Executable file.
EXEC = km

# Builds static kernel.
static:
	$(CC) *.c $(CFLAGS) -rdynamic -D_STATIC_SCHEDULE_ -o $(BINDIR)/$(EXEC).static $(LIBS)

# Cleans compilation files.
clean:
	rm -f $(BINDIR)/$(EXEC).*
