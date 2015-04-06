/*
 * Copyright(C) 2015 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "simulator.h"


/**
 * @brief Static scheduler.
 * 
 * @details Simulates a static loop scheduler.
 */
void scheduler_static(void)
{
	/* Create threads. */
	threads = smalloc(nthreads*sizeof(struct thread));
	for (unsigned i = 0; i < nthreads; i++)
	{
		threads[i].tid = i;
		threads[i].workload = 0;
	}
	
	/* Generate loop iterations. */
	for (unsigned i = 0; i < loop_size; i++)
		/* TODO */ ;
	
	/* Create pool of ready threads. */
	nready = nthreads;
	ready = smalloc(nthreads*sizeof(struct thread *));
	for (unsigned i = 0; i < nthreads; i++)
		ready[i] = &threads[i];
	
	/* Schedule. */
	while (1)
	{
		
		/* Pick a thread to run. */
		while (nready > 0)
		{	
			unsigned i;
		
			i = randnum()%nthreads;
			
			while (read[i] != NULL)
				i = (i + 1)%nthreads;
			nready--;
		}
		
	}
	
	/* Print statistics. */
	for (unsigned i = 0; i < nthreads; i++)
		printf("%u;%u\n", threads[i].tid, threads[i].workload);
	
	/* House keeping. */
	free(ready);
	free(threads);
}
