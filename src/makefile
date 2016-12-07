#
# Copyright(C) 2016 Pedro H. Penna <pedrohenriquepenna@gmail.com>
# 
# This file is part of WorkloadGen.
#
# WorkloadGen is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at
# your option) any later version.
# 
# WorkloadGen is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with WorkloadGen; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.
#

# Builds everything.
all: workloadgen simsched

# Builds WorkloadGen.
workloadgen:                \
		common/workload.o   \
		common/statistics.o \
		workloadgen/main.o       
	mkdir -p $(BINDIR)
	$(LD) $(CFLAGS) $^ -o $(BINDIR)/workloadgen $(LIBS)

# Builds SimSched.
simsched:                   \
		common/workload.o   \
		common/statistics.o \
		simsched/simsched.o \
		simsched/thread.o   \
		simsched/static.o   \
		simsched/main.o
	@mkdir -p $(BINDIR)
	$(LD) $(CFLAGS) $^ -o $(BINDIR)/simsched $(LIBS)

# Builds object file from C source file.
%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@

# Cleans compilation files.
clean:
	@rm -f common/*.o
	@rm -f workloadgen/*.o
	@rm -f simsched/*.o
	@rm -f $(BINDIR)/workloadgen
	@rm -f $(BINDIR)/simsched
