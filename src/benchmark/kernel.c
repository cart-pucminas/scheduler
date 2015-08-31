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

#include <omp.h>
#include <stdbool.h>
#include <string.h>

#include <common.h>
#include <mylib/util.h>

#include "benchmark.h"

/* Workloads. */
unsigned *__tasks; /* Tasks.           */
unsigned __ntasks; /* Number of tasks. */

/**
 * @brief Game of life's boards.
 */
/**@{*/
static unsigned char **boards = NULL;
static unsigned char **sideboards = NULL;
/**@}*/

#define N 64

#define for_x for (unsigned x = 0; x < N; x++)
#define for_y for (unsigned y = 0; y < N; y++)
#define for_xy for_x for_y
#define BOARD(tid, y, x)(boards[tid][(y)*N + (x)])
#define SIDEBOARD(tid, y, x)sideboards[tid][(y)*N + (x)]


/**
 * @brief Play's game of life.
 */
static void play_game(unsigned tid, unsigned niter)
{
	for (unsigned k = 0; k < niter; k++)
	{
		for (unsigned y = 0; y < N; y++)
		{
			for (unsigned x = 0; x < N; x++)
			{
				unsigned n = 0;
				for (unsigned y1 = y - 1; y1 <= y + 1; y1++)
				{
					for (unsigned x1 = x - 1; x1 <= x + 1; x1++)
					{
						if (BOARD(tid,(y1 + N)%N, (x1 + N)%N))
							n++;
					}
				}
		 
				if (BOARD(tid, y, x)) n--;
				SIDEBOARD(tid, y, x) = ((n == 3) || ((n == 2) && BOARD(tid, y, x)));
			}
		}
		
		memcpy(boards[tid], sideboards[tid], N*N*sizeof(unsigned char));
	}
	
}

/**
 * @brief Setups game of life.
 */
static void setup_game(unsigned nthreads)
{	
	/* Create boards. */
	boards = smalloc(nthreads*sizeof(unsigned char *));
	sideboards = smalloc(nthreads*sizeof(unsigned char *));
	
	/* Initialize boards. */
	for (unsigned k = 0; k < nthreads; k++)
	{
		boards[k] = smalloc(N*N*sizeof(unsigned char));
		sideboards[k] = smalloc(N*N*sizeof(unsigned char));
		for (unsigned i = 0; i < N; i++)
		{
			for (unsigned j = 0; j < N; j++)
				BOARD(k, i, j) = (rand() < (RAND_MAX/10)) ? 1 : 0;
		}
	}
}

/**
 * @brief Ends game of life.
 */
static void end_game(unsigned nthreads)
{
	for (unsigned k = 0; k < nthreads; k++)
	{
		free(boards[k]);
		free(sideboards[k]);
	}
	free(boards);
	free(sideboards);
}

/**
 * @brief Simulates a synthetic numeric kernel.
 */
void kernel
(const unsigned *tasks, unsigned ntasks, unsigned nthreads, unsigned scheduler)
{	
	(void) nthreads;
	
	setup_game(nthreads);

	/* Dynamic scheduler. */
	if (scheduler == SCHEDULER_STATIC)
	{
		#pragma omp parallel for schedule(dynamic, chunksize)
		for (unsigned i = 0; i < ntasks; i++)
			play_game(omp_get_thread_num(), tasks[i]);
	}
	
	/* Smart round-robin scheduler. */
	else if (scheduler == SCHEDULER_SMART_ROUND_ROBIN)
	{		
		__ntasks = ntasks;
		__tasks = smalloc(ntasks*sizeof(unsigned));
		memcpy(__tasks, tasks, ntasks*sizeof(unsigned));
		
		#pragma omp parallel for schedule(runtime)
		for (unsigned i = 0; i < ntasks; i++)
			play_game(omp_get_thread_num(), tasks[i]);
		
		free(__tasks);
	}
	
	/* Static scheduler. */
	else
	{
		#pragma omp parallel for schedule(static, chunksize)
		for (unsigned i = 0; i < ntasks; i++)
			play_game(omp_get_thread_num(), tasks[i]);
	}
	
	end_game(nthreads);
}
