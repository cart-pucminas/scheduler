#
# Copyright(C) 2015 Pedro H. Penna <pedrohenriquepenna@gmail.com
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#  
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#  
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA 02110-1301, USA.
#

# Common C source files.
SRC = $(wildcard $(CURDIR)/common/*.c)

# Common object files.
export OBJ = $(SRC:.c=.o)

# Conflicts.
.PHONY: simsched
.PHONY: searcher
.PHONY: generator

# Builds simulator.
simulator: $(OBJ)
	cd simsched && $(MAKE) all
	
# Builds searcher.
searcher: searcher/*.c $(OBJ)
	$(CC) $(CFLAGS) $^ -o $(BINDIR)/searcher $(LIBS)
	
# Builds generator.
generator: generator/*.c $(OBJ)
	$(CC) $(CFLAGS) $^ -o $(BINDIR)/generator $(LIBS)
	
# Builds common objects.
%.o: %.c
	$(CC) $< $(CFLAGS) -c -o $@

# Cleans compilation files.
clean:
	@rm -f $(OBJ)
	cd simsched && $(MAKE) clean
	@rm -f $(BINDIR)/simsched
	@rm -f $(BINDIR)/searcher
	@rm -f $(BINDIR)/generator
