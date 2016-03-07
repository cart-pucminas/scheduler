/*
 * Copyright(C) 2016 Pedro H. Penna <pedrohenriquepenna@gmail.com>
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

#include <stdio.h>
#include <omp.h>

#include "util.h"
#include "mst.h"

unsigned densities[24] = 
{
	1024,
	2048,
	4096,
	8192,
	8192,
	16384,
	32768,
	32768,
	32768,
	32768,
	65536,
	65536,
	1024,
	2048,
	4096,
	8192,
	8192,
	16384,
	32768,
	32768,
	32768,
	32768,
	65536,
	65536
};

unsigned __ntasks;
unsigned *__tasks;


int main(int argc, char **argv)
{
	int n;
	double start, end;
	struct point *data[24];
	
	((void) argc);
	
	n = atoi(argv[1]);
	
	for (int k = 0; k < 24; k++)
	{
		data[k] = smalloc(densities[k]*sizeof(struct point));
		
		/* Initialize data set. */
		for (unsigned i = 0; i < densities[k]; i++)
		{
			data[k][i].x = rand()%10000;
			data[k][i].y = rand()%10000;
		}
	}
	
	start = omp_get_wtime();
#if defined(_SCHEDULE_STATIC_)
	#pragma omp parallel for schedule(static) num_threads(n) default(shared)
#elif defined(_SCHEDULE_DYNAMIC_)
	#pragma omp parallel for schedule(dynamic) num_threads(n) default(shared)
#elif defined(_SCHEDULE_SRR_)
	__tasks = densities;
	__ntasks = 24;
	#pragma omp parallel for schedule(runtime) num_threads(n) default(shared)
#endif
	for (int i = 0; i < 24; i++)
		mst(data[i], densities[i]);
	end = omp_get_wtime();
	
	printf("npoints: %d\n", n);
	printf("memory: %u\n", (unsigned)(n*sizeof(struct point)));
	printf("time: %lf\n", end - start);
	
	/* House keeping. */
	for (int i = 0; i < 24; i++)
		free(data[i]);
	
	return (EXIT_SUCCESS);
}
