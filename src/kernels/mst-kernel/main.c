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
#include <string.h>
#include <papi.h>

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
	16384,
	32768,
	32768,
	32768,
	65536,
	1024,
	2048,
	4096,
	8192,
	8192,
	16384,
	16384,
	32768,
	32768,
	32768,
	65536,
	65536,
	65536
};

static int trace = 0;

unsigned __ntasks;
unsigned *__tasks;


int main(int argc, char **argv)
{
	int n;
	double start, end;
	struct point *data[24];
	int events[4] = { PAPI_L1_DCM, PAPI_L2_DCM, PAPI_L2_DCA, PAPI_L3_DCA };
	long long hwcounters[4];
#ifdef _SCHEDULE_ORACLE_
	unsigned taskmap[24] = {7,  8, 10, 1, 6,  1, 10, 11, 6, 8, 0, 8,
		                    7, 11,  3, 2, 1, 10,  2,  3, 7, 5, 4, 9};
		            
#endif

	__tasks=smalloc(24*sizeof(unsigned));

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

	if (PAPI_start_counters(events, 4) != PAPI_OK)
		error("failed to setup PAPI");
	
	start = omp_get_wtime();
#if defined(_SCHEDULE_STATIC_)
	#pragma omp parallel for schedule(static) num_threads(n) default(shared)
#elif defined(_SCHEDULE_DYNAMIC_)
	#pragma omp parallel for schedule(dynamic) num_threads(n) default(shared)
#elif defined(_SCHEDULE_SRR_)
	memcpy(__tasks, densities, 24*sizeof(unsigned));
	__ntasks = 24;
	#pragma omp parallel for schedule(runtime) num_threads(n) default(shared)
#elif defined(_SCHEDULE_ORACLE_)
	memcpy(__tasks, taskmap, 24*sizeof(unsigned));
	__ntasks = 24;
	#pragma omp parallel for schedule(runtime) num_threads(n) default(shared)
#endif
	for (int i = 0; i < 24; i++)
	{
		if (trace)
		{
			#pragma omp critical
			printf("thread %d timestamp %lf\n", omp_get_thread_num(), omp_get_wtime());
		}
		mst(data[i], densities[i]);
		if (trace)
		{
			#pragma omp critical
			printf("thread %d timestamp %lf\n", omp_get_thread_num(), omp_get_wtime());
		}
	}
	end = omp_get_wtime();

	if (PAPI_stop_counters(hwcounters, sizeof(events)) != PAPI_OK)
		error("failed to read hardware counters");
	
	printf("time: %lf\n", end - start);
	printf("L1 Misses: %lld\n", hwcounters[0]);
	printf("L2 Misses: %lld\n", hwcounters[1]);
	printf("L2 Accesses: %lld\n", hwcounters[2]);
	printf("L3 Accesses: %lld\n", hwcounters[3]);
	
	/* House keeping. */
	for (int i = 0; i < 24; i++)
		free(data[i]);
	free(__tasks);
	
	return (EXIT_SUCCESS);
}
