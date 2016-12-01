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
#include <util.h>

/*====================================================================*
 * HISTOGRAM                                                          *
 *====================================================================*/

/**
 * @brief Histogram.
 */
struct histogram
{
	int nclasses;    /**< Number of classes. */
	double *classes; /**< classes            */
};

/**
 * @brief Creates a histogram.
 *
 * @param nclasses Number of classes.
 *
 * @returns A histogram.
 */
static struct histogram *histogram_create(int nclasses)
{
	struct histogram *h;
	
	/* Sanity check. */
	assert(nclasses > 0);

	/* Create histogram. */
	h = smalloc(sizeof(struct histogram));
	h->nclasses = nclasses;
	h->classes = smalloc(nclasses*sizeof(double));

	return (h);
}

/**
 * @brief Destroys a histogram.
 *
 * @param h Target histogram.
 */
void histogram_destroy(const struct histogram *h)
{
	/* Sanity check, */
	assert(h != NULL);

	free((void *) h->classes);
	free((void *) h);
}

/**
 * @brief Returns the number of classes in a histogram.
 *
 * @param h Target histogram.
 *
 * @returns The number of classes in a histogram.
 */
int histogram_nclasses(const struct histogram *h)
{
	/* Sanity check. */
	assert(h != NULL);

	return (h->nclasses);
}

/**
 * @brief Returns the frequency of a class in a histogram.
 *
 * @param h Target histogram.
 * @param i Target class.
 *
 * @returns The frequency of a class in a histogram.
 */
double histogram_class(const struct histogram *h, int i)
{
	/* Sanity check. */
	assert(h != NULL);
	assert((i > 0) && (i < h->nclasses));

	return (h->classes[i]);
}

/*====================================================================*
 * PROBABILITY DISTRIBUTION                                           *
 *====================================================================*/

/**
 * @brief Probability distribution.
 */
struct distribution
{
	double kurtosis;                      /**< Distribution's kurtosis.  */
	histogram_tt (*histgen)(double, int); /**< Histogram generator.      */
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
const struct histogram *histgen(const struct distribution *dist, int nclasses)
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
static const struct histogram *beta_histgen(double kurtosis, int nclasses)
{
	double sum;          /* PDF sum.         */
	double freq;         /* Class frequency. */
	struct histogram *h; /* Histogram.       */

	h = histogram_create(nclasses);

	/* Build histogram. */
	freq = 0.5; sum = 0.0;
	for (int i = 0; i < nclasses/2; i++)
	{
		if (i < (4*nclasses)/12)
			freq *= kurtosis;
		else
		freq *= 0.95;

		h->classes[i] = freq;
		h->classes[nclasses - i - 1] = freq;

		sum	+= freq	+ freq;
	}

	/* Normalize. */
	for (int i = 0; i < nclasses; i++)
		h->classes[i] /= sum;

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
static const struct histogram *gamma_histgen(double kurtosis, int nclasses)
{
	double sum;          /* PDF sum.         */
	double freq;         /* Class frequency. */
	struct histogram *h; /* Histogram.       */

	h = histogram_create(nclasses);

	/* Build histogram. */
	freq = 1.0; sum = 0.0;
	for (int i = 0; i < nclasses; i++)
	{
		freq *= kurtosis;

		h->classes[i] = freq;

		sum += freq;
	}

	/* Normalize. */
	for (int i = 0; i < nclasses; i++)
		h->classes[i] /= sum;

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
static const struct histogram *gaussian_histgen(double kurtosis, int nclasses)
{
	double sum;          /* PDF sum.         */
	double freq;         /* Class frequency. */
	struct histogram *h; /* Histogram.       */

	h = histogram_create(nclasses);

	/* Build histogram. */
	freq = 0.5; sum = 0.0;
	for (int i = 0; i < nclasses/2; i++)
	{
		freq *= kurtosis;

		h->classes[nclasses/2 - i - 1] = freq;
		h->classes[nclasses/2 + i] = freq;

		sum += freq + freq;
	}

	/* Normalize. */
	for (int i = 0; i < nclasses; i++)
		h->classes[i] /= sum;

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
static const struct histogram *uniform_histgen(double kurtosis, int nclasses)
{
	double freq;         /* Class frequency. */
	struct histogram *h; /* Histogram.       */

	((void) kurtosis);

	h = histogram_create(nclasses);

	/* Build histogram. */
	freq = 1.0/nclasses;
	for (int i = 0; i < nclasses; i += 2)
	{
		h->classes[i] = freq;
		h->classes[i + 1] = freq;
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
