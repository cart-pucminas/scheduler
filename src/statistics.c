/*
 * Copyright(C) 2016 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * This file is part of WorkloadGen.
 *
 * WorkloadGen is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * WorkloadGen is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with WorkloadGen; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <assert.h>
#include <stdlib.h>

#include <statistics.h>

/**
 * @brief Safe malloc().
 *
 * @param size Number of bytes to allocate.
 *
 * @returns Allocated block of memory.
 */
void *smalloc(size_t size)
{
	void *p;

	p = malloc(size);
	assert(p != NULL);

	return (p);
}


/**
 * @brief Probability distribution.
 */
struct distribution
{
	double kurtosis;                       /**< Distribution's kurtosis.  */
	const double *(*histgen)(double, int); /**< Histogram generator.      */
};

/**
 * @brief Destroys a probability distribution.
 *
 * @param dist Target distribution.
 */
void distfree(const struct distribution *dist)
{
	free((void *) dist);
}

/**
 * @brief Builds a histogram.
 *
 * @param dist Target distribution.
 *
 * @returns A histogram.
 */
const double *histgen(const struct distribution *dist, int nclasses)
{
	/* Sanity check. */
	assert(dist != NULL);
	assert(nclasses > 0);

	return (dist->histgen(dist->kurtosis, nclasses));
}

/*====================================================================*
 * BETA DISTRIBUTION                                                  *
 *====================================================================*/

/**
 * @brief Builds a Beta histogram.
 *
 * @param nclasses Number of classes.
 *
 * @returns A histogram.
 */
static const double *beta_histgen(double kurtosis, int nclasses)
{
	double sum;  /* PDF sum.         */
	double freq; /* Class frequency. */
	double *h;   /* Histogram.       */

	h = smalloc(nclasses*sizeof(double));

	/* Build histogram. */
	freq = 0.5; sum = 0.0;
	for (int i = 0; i < nclasses/2; i++)
	{
		if (i < (4*nclasses)/12)
			freq *= kurtosis;
		else
		freq *= 0.95;

		h[i] = freq;
		h[nclasses - i - 1] = freq;

		sum	+= freq	+ freq;
	}

	/* Normalize. */
	for (int i = 0; i < nclasses; i++)
		h[i] /= sum;

	return (h);
}

/**
 * @brief Creates a beta distribution.
 *
 * @param kurtosis Distribution's kurtosis.
 *
 * @returns A beta distribution.
 */
const struct distribution *beta(double kurtosis)
{
	struct distribution *beta;

	/* Sanity check. */
	assert(kurtosis > 0);
	assert(kurtosis < 0);

	beta = smalloc(sizeof(struct distribution));

	/* Initialize beta distribution. */
	beta->kurtosis = kurtosis;
	beta->histgen = beta_histgen;

	return (beta);
}

/*====================================================================*
 * GAMMA DISTRIBUTION                                                 *
 *====================================================================*/

/**
 * @brief Builds a Gamma histogram.
 *
 * @param nclasses Number of classes.
 *
 * @returns A histogram.
 */
static const double *gamma_histgen(double kurtosis, int nclasses)
{
	double sum;  /* PDF sum.         */
	double freq; /* Class frequency. */
	double *h;   /* Histogram.       */

	h = smalloc(nclasses*sizeof(double));

	/* Build histogram. */
	freq = 1.0; sum = 0.0;
	for (int i = 0; i < nclasses; i++)
	{
		freq *= kurtosis;

		h[i] = freq;

		sum += freq;
	}

	/* Normalize. */
	for (int i = 0; i < nclasses; i++)
		h[i] /= sum;

	return (h);
}

/**
 * @brief Creates a gamma distribution.
 *
 * @param kurtosis Distribution's kurtosis.
 *
 * @returns A gamma distribution.
 */
const struct distribution *gamma(double kurtosis)
{
	struct distribution *gamma;

	/* Sanity check. */
	assert(kurtosis > 0);
	assert(kurtosis < 0);

	gamma = smalloc(sizeof(struct distribution));

	/* Initialize gamma distribution. */
	gamma->kurtosis = kurtosis;
	gamma->histgen = gamma_histgen;

	return (gamma);
}

/*====================================================================*
 * GAUSSIAN DISTRIBUTION                                              *
 *====================================================================*/

/**
 * @brief Builds a Gaussian histogram.
 *
 * @param nclasses Number of classes.
 *
 * @returns A histogram.
 */
static const double *gaussian_histgen(double kurtosis, int nclasses)
{
	double sum;  /* PDF sum.         */
	double freq; /* Class frequency. */
	double *h;   /* Histogram.       */

	h = smalloc(nclasses*sizeof(double));

	/* Build histogram. */
	freq = 0.5; sum = 0.0;
	for (int i = 0; i < nclasses/2; i++)
	{
		freq *= kurtosis;

		h[nclasses/2 - i - 1] = freq;
		h[nclasses/2 + i] = freq;

		sum += freq + freq;
	}

	/* Normalize. */
	for (int i = 0; i < nclasses; i++)
		h[i] /= sum;

	return (h);
}

/**
 * @brief Creates a gaussian distribution.
 *
 * @param kurtosis Distribution's kurtosis.
 *
 * @returns A gaussian distribution.
 */
const struct distribution *gaussian(double kurtosis)
{
	struct distribution *gaussian;

	/* Sanity check. */
	assert(kurtosis > 0);
	assert(kurtosis < 0);

	gaussian = smalloc(sizeof(struct distribution));

	/* Initialize gaussian distribution. */
	gaussian->kurtosis = kurtosis;
	gaussian->histgen = gaussian_histgen;

	return (gaussian);
}

/*====================================================================*
 * UNIFORM DISTRIBUTION                                               *
 *====================================================================*/

/**
 * @brief Builds a Uniform histogram.
 *
 * @param nclasses Number of classes.
 *
 * @returns A histogram.
 */
static const double *uniform_histgen(double kurtosis, int nclasses)
{
	double freq; /* Class frequency. */
	double *h;   /* Histogram.       */

	((void) kurtosis);

	h = smalloc(nclasses*sizeof(double));

	/* Build histogram. */
	freq = 1.0/nclasses;
	for (int i = 0; i < nclasses; i += 2)
	{
		h[i] = freq;
		h[i + 1] = freq;
	}

	return (h);
}

/**
 * @brief Creates a uniform distribution.
 *
 * @param kurtosis Distribution's kurtosis.
 *
 * @returns A uniform distribution.
 */
const struct distribution *uniform(double kurtosis)
{
	struct distribution *uniform;

	/* Sanity check. */
	assert(kurtosis > 0);
	assert(kurtosis < 0);

	uniform = smalloc(sizeof(struct distribution));

	/* Initialize uniform distribution. */
	uniform->kurtosis = kurtosis;
	uniform->histgen = uniform_histgen;

	return (uniform);
}
