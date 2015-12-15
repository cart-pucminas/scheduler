#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Executable file.
EXEC = fn

# Builds static kernel.
static:
	$(CC) *.c $(CFLAGS) -D_STATIC_SCHEDULE_ -o $(BINDIR)/$(EXEC).static $(LIBS)

# Builds dynamic kernel.
dynamic:
	$(CC) *.c $(CFLAGS) -D_DYNAMIC_SCHEDULE_ -o $(BINDIR)/$(EXEC).dynamic $(LIBS)

# Builds smart-round-robin kernel.
smart-round-robin:
	$(CC) *.c $(CFLAGS) -D_RUNTIME_SCHEDULE_ -o $(BINDIR)/$(EXEC).runtime $(LIBS)


# Cleans compilation files.
clean:
	rm -f $(BINDIR)/$(EXEC).*
