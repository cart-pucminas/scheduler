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

#include <mylib/util.h>

#include <gsl/gsl_randist.h>

#include <statistics.h>

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
void histogram_destroy(struct histogram *h)
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
	assert((i >= 0) && (i < h->nclasses));

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
void distribution_destroy(struct distribution *dist)
{
	free(dist);
}

/**
 * @brief Builds a histogram.
 *
 * @param dist Target distribution.
 *
 * @returns A histogram.
 */
struct histogram *distribution_histogram(const struct distribution *dist, int nclasses)
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
static struct histogram *beta_histgen(double kurtosis, int nclasses)
{
	struct histogram *h;  /* Histogram. */
	const double a = 0.5; /* Alpha.     */
	const double b = 0.5; /* Beta.      */
	double density = 0.0; /* Density.   */

	((void) kurtosis);

	h = histogram_create(nclasses);

	/* Build histogram. */
	for (int i = 0; i < nclasses; i++)
	{
		double x = 0.05 + i*0.9/(nclasses - 1);

		h->classes[i] = gsl_ran_beta_pdf(x, a, b);
		density += h->classes[i];
	}

	/* Normalize. */
	if (density > 1.0)
	{
		for (int i = 0; i < nclasses; i++)
			h->classes[i] /= density;
	}

	return (h);
}

/**
 * @brief Creates a beta distribution.
 *
 * @param kurtosis Distribution's kurtosis.
 *
 * @returns A beta distribution.
 */
struct distribution *dist_beta(double kurtosis)
{
	struct distribution *beta;

	/* Sanity check. */
	assert((kurtosis > 0) && (kurtosis < 1.0));

	beta = smalloc(sizeof(struct distribution));

	/* Initialize beta distribution. */
	beta->kurtosis = kurtosis;
	beta->histgen = beta_histgen;

	return (beta);
}

/*====================================================================*
 * EXPONENTIAL DISTRIBUTION                                           *
 *====================================================================*/

/**
 * @brief Builds a Exponential histogram.
 *
 * @param nclasses Number of classes.
 *
 * @returns A histogram.
 */
static struct histogram *exponential_histgen(double kurtosis, int nclasses)
{
	struct histogram *h;   /* Histogram. */
	const double mu = 5.0; /* Mean.     */
	double density = 0.0;  /* Density.   */

	((void) kurtosis);

	h = histogram_create(nclasses);

	/* Build histogram. */
	for (int i = 0; i < nclasses; i++)
	{
		double x = 0.0 + i*12.0/(nclasses - 1);

		h->classes[i] = gsl_ran_exponential_pdf(x, mu);
		density += h->classes[i];
	}

	/* Normalize. */
	if (density > 1.0)
	{
		for (int i = 0; i < nclasses; i++)
			h->classes[i] /= density;
	}

	return (h);
}

/**
 * @brief Creates a exponential distribution.
 *
 * @param kurtosis Distribution's kurtosis.
 *
 * @returns A exponential distribution.
 */
struct distribution *dist_exponential(double kurtosis)
{
	struct distribution *exponential;

	/* Sanity check. */
	assert((kurtosis > 0) && (kurtosis < 1.0));

	exponential = smalloc(sizeof(struct distribution));

	/* Initialize exponential distribution. */
	exponential->kurtosis = kurtosis;
	exponential->histgen = exponential_histgen;

	return (exponential);
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
static struct histogram *gamma_histgen(double kurtosis, int nclasses)
{
	struct histogram *h;      /* Histogram. */
	const double k = 5.0;     /* Shape.     */
	const double theta = 1.0; /* Scale.     */
	double density = 0.0;     /* Density.   */

	((void) kurtosis);

	h = histogram_create(nclasses);

	/* Build histogram. */
	for (int i = 0; i < nclasses; i++)
	{
		double x = 0.0 + i*12.0/(nclasses - 1);

		h->classes[i] = gsl_ran_gamma_pdf(x, k, theta);
		density += h->classes[i];
	}

	/* Normalize. */
	if (density > 1.0)
	{
		for (int i = 0; i < nclasses; i++)
			h->classes[i] /= density;
	}

	return (h);
}

/**
 * @brief Creates a gamma distribution.
 *
 * @param kurtosis Distribution's kurtosis.
 *
 * @returns A gamma distribution.
 */
struct distribution *dist_gamma(double kurtosis)
{
	struct distribution *gamma;

	/* Sanity check. */
	assert((kurtosis > 0) && (kurtosis < 1.0));

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
static struct histogram *gaussian_histgen(double kurtosis, int nclasses)
{
	struct histogram *h;      /* Histogram.          */
	const double sigma = 1.0; /* Standard deviation. */
	double density = 0.0;     /* Density.            */

	((void) kurtosis);

	h = histogram_create(nclasses);

	/* Build histogram. */
	for (int i = 0; i < nclasses; i++)
	{
		double x = -2.5 + i*5.0/(nclasses - 1);

		h->classes[i] = gsl_ran_gaussian_pdf(x, sigma);
		density += h->classes[i];
	}

	/* Normalize. */
	if (density > 1.0)
	{
		for (int i = 0; i < nclasses; i++)
			h->classes[i] /= density;
	}

	return (h);
}

/**
 * @brief Creates a gaussian distribution.
 *
 * @param kurtosis Distribution's kurtosis.
 *
 * @returns A gaussian distribution.
 */
struct distribution *dist_gaussian(double kurtosis)
{
	struct distribution *gaussian;

	/* Sanity check. */
	assert((kurtosis > 0) && (kurtosis < 1.0));

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
static struct histogram *uniform_histgen(double kurtosis, int nclasses)
{
	struct histogram *h;  /* Histogram. */
	const double a = 0.0; /* Min.       */
	const double b = 1.0; /* Max.       */
	double density = 0.0; /* Density.   */

	((void) kurtosis);

	h = histogram_create(nclasses);

	/* Build histogram. */
	for (int i = 0; i < nclasses; i++)
	{
		double x = 0.05 + i*0.9/(nclasses - 1);

		h->classes[i] = gsl_ran_flat_pdf(x, a, b);
		density += h->classes[i];
	}

	/* Normalize. */
	if (density > 1.0)
	{
		for (int i = 0; i < nclasses; i++)
			h->classes[i] /= density;
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
struct distribution *dist_uniform(double kurtosis)
{
	struct distribution *uniform;

	/* Sanity check. */
	assert((kurtosis > 0) && (kurtosis < 1.0));

	uniform = smalloc(sizeof(struct distribution));

	/* Initialize uniform distribution. */
	uniform->kurtosis = kurtosis;
	uniform->histgen = uniform_histgen;

	return (uniform);
}
