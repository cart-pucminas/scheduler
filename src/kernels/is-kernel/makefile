#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Executable file.
EXEC = is

# Builds static version.
static:
	$(CC) *.c $(CFLAGS) -rdynamic -D_SCHEDULE_STATIC_ -o $(BINDIR)/$(EXEC).static $(LIBS)

# Builds dynamic version.
dynamic:
	$(CC) *.c $(CFLAGS) -rdynamic -D_SCHEDULE_DYNAMIC_ -o $(BINDIR)/$(EXEC).dynamic $(LIBS)

# Cleans compilation files.
clean:
	rm -f $(BINDIR)/$(EXEC).*
