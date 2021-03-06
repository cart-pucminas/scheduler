/*
 * Copyright(C) 2016 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * This file is part of Scheduler.
 *
 * Scheduler is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * Scheduler is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Scheduler; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>

#include <mylib/util.h>

#include <statistics.h>
#include <workload.h>

/**
 * @brief Synthetic workload.
 */
struct workload
{
	int ntasks; /**< Number of tasks. */
	int *tasks; /**< Tasks.           */
};

/**
 * @brief Computes the skewness of a task.
 *
 * @param i        Task class.
 * @param nclasses Number of task classes
 * @param skewness Skewness.
 *
 * @returns Task skewness.
 */
static int workload_skewness(int i, int nclasses, int skewness)
{
	switch (skewness)
	{
		case WORKLOAD_SKEWNESS_LEFT:
			return (i + 2);
			break;

		case WORKLOAD_SKEWNESS_RIGHT:
			return (nclasses - i + 1);
			break;

		default:
			error("unknown skewness");
	}
	
	/* Never gets here. */
	return (-1);
}

/**
 * @brief Creates a workload.
 *
 * @param h        Histogram of probability distribution.
 * @param skewness Skewness.
 * @param ntasks   Number of tasks.
 */
struct workload *workload_create(histogram_tt h, int skewness, int ntasks)
{
	int k;              /* Residual tasks. */
	struct workload *w; /* Workload.       */

	/* Sanity check. */
	assert(h != NULL);
	assert(ntasks > 0);

	/* Create workload. */
	w = smalloc(sizeof(struct workload));
	w->ntasks = ntasks;
	w->tasks = smalloc(ntasks*sizeof(int));

	/* Create workload. */
	k = 0;
	for (int i = 0; i < histogram_nclasses(h); i++)
	{
		int n;

		n = floor(histogram_class(h, i)*ntasks);

		for (int j = 0; j < n; j++)
			w->tasks[k++] = workload_skewness(i, histogram_nclasses(h), skewness);
	}

	/* Check for overflow. */
	if (k > ntasks)
	{
		fprintf(stderr, "ntasks=%d\n", k);
		error("histogram overflow");
	}

	/* Fill up remainder tasks. */
	for (int i = k; i < ntasks; i++)
	{
		int j = rand()%histogram_nclasses(h);
		w->tasks[k++] = workload_skewness(j, histogram_nclasses(h), skewness);
	}

	return (w);
}

/**
 * @brief Destroys a workload.
 *
 * @param w Target workload.
 */
void workload_destroy(struct workload *w)
{
	/* Sanity check. */
	assert(w != NULL);

	free(w->tasks);
	free(w);
}

/**
 * @brief Sorts tasks in ascending order.
 *
 * @oaram w Target workload.
 */
static void workload_ascending(struct workload *w)
{
	/* Sanity check. */
	assert(w != NULL);

	for (int i = 0; i < w->ntasks; i++)
	{
		for (int j = 0; j < w->ntasks; j++)
		{
			if (w->tasks[j] < w->tasks[i])
			{
				double tmp;

				tmp = w->tasks[j];
				w->tasks[j] = w->tasks[i];
				w->tasks[i] = tmp;
			}
		}
	}
}

/**
 * @brief Sorts tasks in descending order.
 *
 * @oaram w Target workload.
 */
static void workload_descending(struct workload *w)
{
	/* Sanity check. */
	assert(w != NULL);

	for (int i = 0; i < w->ntasks; i++)
	{
		for (int j = 0; j < w->ntasks; j++)
		{
			if (w->tasks[j] > w->tasks[i])
			{
				double tmp;

				tmp = w->tasks[j];
				w->tasks[j] = w->tasks[i];
				w->tasks[i] = tmp;
			}
		}
	}
}

/**
 * @brief Shuffle tasks.
 *
 * @oaram w Target workload.
 */
static void workload_shuffle(struct workload *w)
{
	/* Sanity check. */
	assert(w != NULL);

	/* Shuffle array. */
	for (int i = 0; i < w->ntasks - 1; i++)
	{
		int j;      /* Shuffle index.  */
		double tmp; /* Temporary data. */

		j = rand()%w->ntasks;

		tmp = w->tasks[i];
		w->tasks[i] = w->tasks[j];
		w->tasks[j] = tmp;
	}
}

/**
 * @brief Sorts tasks.
 *
 * @param w       Target workload.
 * @param sorting Sorting type.
 */
void workload_sort(struct workload *w, enum workload_sorting sorting)
{
	/* Sanity check. */
	assert(w != NULL);

	/* Sort workload. */
	switch(sorting)
	{
		/* Sort in ascending order. */
		case WORKLOAD_ASCENDING:
			workload_ascending(w);
			break;

		/* Sort in descending order. */
		case WORKLOAD_DESCENDING:
			workload_descending(w);
			break;

		/* Shuffle workload. */
		case WORKLOAD_SHUFFLE:
			workload_shuffle(w);
			break;

		/* Should not happen. */
		default:
			assert(0);
			break;
	}
}
/**
 * @brief Computes the task sorting map of a workload.
 * 
 * @param w Target workload
 * 
 * @returns Sorting map.
 */
int *workload_sortmap(const struct workload *w)
{
	int *map;

	/* Sanity check. */
	assert(w != NULL);
	
	map = smalloc(w->ntasks*sizeof(int));
	for (int i = 0; i < w->ntasks; i++)
		map[i] = i;

	/* Sort. */
	for (int i = 0; i < w->ntasks - 1; i++)
	{
		for (int j = i + 1; j < w->ntasks; j++)
		{
			/* Swap. */
			if (w->tasks[map[j]] < w->tasks[map[i]])
			{
				int tmp;
				
				tmp = map[j];
				map[j] = map[i];
				map[i] = tmp;
			}
		}
	}
	
	return (map);
} 

/**
 * @brief Writes a workload to a file.
 *
 * @param outfile Output file.
 * @param w       Target workload.
 */
void workload_write(FILE *outfile, const struct workload *w)
{
	/* Sanity check. */
	assert(outfile != NULL);
	assert(w != NULL);

	/* Write workload to file. */
	fprintf(outfile, "%d\n", w->ntasks);
	for (int i = 0; i < w->ntasks; i++)
		fprintf(outfile, "%d\n", w->tasks[i]);
}

/**
 * @brief Reads a workload from a file.
 *
 * @param infile Input file.
 *
 * @returns A workload.
 */
struct workload *workload_read(FILE *infile)
{
	int ntasks;         /**< Number of tasks. */
	struct workload *w; /**< Workload.        */

	/* Sanity check. */
	assert(infile != NULL);

	assert(fscanf(infile, "%d\n", &ntasks) == 1);

	w = smalloc(sizeof(struct workload));
	w->tasks = smalloc(ntasks*sizeof(int));
	w->ntasks = ntasks;

	/* Write workload to file. */
	for (int i = 0; i < ntasks; i++)
		assert(fscanf(infile, "%d\n", &w->tasks[i]) == 1);

	return (w);
}

/**
 * @brief Returns the number of tasks in a workload.
 *
 * @param w Target workload.
 *
 * @returns The number of tasks in the target workload.
 */
int workload_ntasks(const struct workload *w)
{
	/* Sanity check. */
	assert(w != NULL);

	return (w->ntasks);
}

/**
 * @brief Returns ith task in a workload.
 *
 * @param w   Target workload.
 * @param idx Index of  target task.
 *
 * @returns The ith task in the target workload.
 */
int workload_task(const struct workload *w, int idx)
{
	/* Sanity check. */
	assert(w != NULL);
	assert((idx >= 0) && (idx < w->ntasks));

	return (w->tasks[idx]);
}

/**
 * @brief Adjusts the load of the ith task in a workload.
 *
 * @param w    Target workload.
 * @param idx  Index of target task.
 * @param load New load.
 */
void workload_set_task(struct workload *w, int idx, int load)
{
	/* Sanity check. */
	assert(w != NULL);
	assert((idx >= 0) && (idx < w->ntasks));
	assert(load > 0);

	w->tasks[idx] = load;
}

/**
 * @brief Computes the cummulative sum of a workload.
 *
 * @param w Target Workload.
 *
 * @returns The cummulative sum array.
 */
int *workload_cummulative_sum(const struct workload *w)
{
	int *sum;   /* Cummulative sum.     */
	int ntasks; /* Alias for w->ntasks. */

	/* Sanity check. */
	assert(w != NULL);

	ntasks = w->ntasks;

	sum = malloc((ntasks + 1)*sizeof(int));
	assert(sum != NULL);

	/* Compute cummulative sum. */
	sum[0] = 0;
	for (int i = 1; i <= ntasks; i++)
		sum[i] = sum[i - 1] + w->tasks[i - 1];

	return (sum);

}
