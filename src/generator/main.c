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

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <mylib/util.h>
#include <pdf.h>

/**
 * @brief Supported probability density functions.
 */
/**@{*/
#define NR_PDFS      4 /**< Number of supported PDFs. */
#define PDF_BETA     1 /**< Beta.                     */
#define PDF_GAMMA    2 /**< Gamma.                    */
#define PDF_GAUSSIAN 3 /**< Gaussian.                 */
#define PDF_POISSON  4 /**< Poisson.                  */
/**@}*/

/**
 * @brief Name of supported probability density functions.
 */
static const char *pdfnames[NR_PDFS] = {
	"beta",     /* Beta.     */
	"gamma",    /* Gammma.   */
	"gaussian", /* Gaussian. */
	"poisson"   /* Poisson.  */
};

/**
 * @brief Number of supported kernels.
 */
#define NR_KERNELS 3

/**
 * @brief Name of supported kernel types.
 */
/**@{*/
#define KERNEL_LINEAR      1 /**< Linear kernel O(n).            */
#define KERNEL_LOGARITHMIC 2 /**< Logarithmic kernel O(n log n). */
#define KERNEL_QUADRATIC   3 /**< Quadratic kernel O(n^2).       */
/**@}*/

/**
 * @brief Name of supported kernel types.
 */
static const char *kernelnames[NR_KERNELS] = {
	"linear",    /* Linear.    */
	"logarithm", /* Logarithm. */
	"quadratic"  /* Quadratic. */
};

/**
 * @name Program Parameters
 */
static struct
{
	int kernel;      /**< Kernel type.                           */
	int ntasks;      /**< Number of tasks.                       */
	int pdf;         /**< Probability density function.          */
	double skewness; /**< Probability density function skewness. */
	int nclasses;    /**< Number of task classes.                */
} args = { 0, 0, 0, 0.0 , 0};

/**
 * @brief Prints program usage and exits.
 * 
 * @details Prints program usage and exits gracefully.
 */
static void usage(void)
{
	printf("Usage: generator [options]\n");
	printf("Brief: workload generator\n");
	printf("Options:\n");
	printf("  --nclasses <number> Number of task classes.\n");
	printf("  --ntasks <number>   Number tasks.\n");
	printf("  --kernel <name>     Kernel type.\n");
	printf("           linear     Linear O(n)\n");
	printf("           logarithm  Logarithm O(n log n)\n");
	printf("           quadratic  Quadratic O(n^2)\n");
	printf("  --pdf <name>        Probability desity function for random numbers.\n");
	printf("        beta            a = 0.5 and b = 0.5\n");
	printf("        gamma           a = 1.0 and b = 2.0 \n");
	printf("        gaussian        x = 0.0 and std = 1.0\n");
	printf("        poisson                                \n");
	printf("  --skewness <number> PDF skewness.\n");
	printf("  --help              Display this message.\n");

	exit(EXIT_SUCCESS);
}

/*============================================================================*
 *                                Get Routines                                *
 *============================================================================*/

/**
 * @brief Gets PDF id.
 * 
 * @param pdfname PDF name.
 * 
 * @returns PDF id.
 */
static int getpdf(const char *pdfname)
{
	for (int i = 0; i < NR_PDFS; i++)
	{
		if (!strcmp(pdfname, pdfnames[i]))
			return (i + 1);
	}
	
	error("unsupported probability density function");
	
	/* Never gets here. */
	return (-1);
}

/**
 * @brief Gets kernel type.
 * 
 * @param kernelname Kernel name.
 * 
 * @returns Kernel type.
 */
static int getkernel(const char *kernelname)
{
	for (int i = 0; i < NR_KERNELS; i++)
	{
		if (!strcmp(kernelname, kernelnames[i]))
			return (i + 1);
	}
	
	error("unsupported kernel type");
	
	/* Never gets here. */
	return (-1);
}

/*============================================================================*
 *                             Argument Checking                              *
 *============================================================================*/

/**
 * @brief Checks program arguments.
 */
static void checkargs(const char *pdfname, const char *kernelname)
{
	/* Check parameters. */
	if (!(args.ntasks > 0))
		error("invalid number of iterations in the parallel loop");
	else if (pdfname == NULL)
		error("missing probability density function");
	else if (!(args.skewness > 0.1))
		error("invalid skewness for probability density function");
	else if (kernelname == NULL)
		error("invalid kernel type");
	else if (!(args.nclasses > 0))
		error("invalid number of task classes");
}

/**
 * @brief Reads command line arguments.
 * 
 * @details Reads command line arguments.
 */
static void readargs(int argc, const char **argv)
{
	const char *pdfname = NULL;
	const char *kernelname = NULL;
	
	/* Parse command line arguments. */
	for (int i = 1; i < argc; i++)
	{	
		/* Parse command. */
		if (!strcmp(argv[i], "--ntasks"))
			args.ntasks = atoi(argv[++i]);
		else if (!strcmp(argv[i], "--pdf"))
			pdfname = argv[++i];
		else if (!strcmp(argv[i], "--skewness"))
			args.skewness = atof(argv[++i]);
		else if (!strcmp(argv[i], "--kernel"))
			kernelname = argv[++i];
		else if (!strcmp(argv[i], "--help"))
			usage();
		else if (!strcmp(argv[i], "--nclasses"))
			args.nclasses = atoi(argv[++i]);
	}
	
	checkargs(pdfname, kernelname);
	
	args.pdf = getpdf(pdfname);
	args.kernel = getkernel(kernelname);
}

/*============================================================================*
 *                           WORKLOAD GENERATOR                               *
 *============================================================================*/

/**
 * @brief Builds tasks histogram.
 * 
 * @param pdf      Probability density functions.
 * @param nclasses Number of task classes.
 * @param skewness Skewness for probability density function.
 * 
 * @returns tasks histogram.
 */
static double *histogram_create(unsigned pdf, int nclasses, double skewness)
{
	double *h = NULL;
	
	/* Generate input data. */
	switch (pdf)
	{
		/* Beta distribution. */
		case PDF_BETA:
			h = beta(nclasses, skewness);
			break;
			
		/* Gamma distribution. */
		case PDF_GAMMA:
			h = gamma(nclasses, skewness);
			break;
			
		/* Gaussian distribution. */
		case PDF_GAUSSIAN:
			h = gaussian(nclasses, skewness);
			break;
			
		/* Poisson distribution. */
		case PDF_POISSON:
			h = poisson(nclasses, skewness);
			break;
			
		/* Shouldn't happen. */
		default:
			error("unsupported probability density function");
			break;
	}
	
	return (h);
}

/**
 * @brief Create tasks.
 * 
 * @param h        Tasks histogram.
 * @param nclasses Number of task classes.
 * @param ntasks   Number of tasks.
 * 
 * @returns Tasks.
 */
static unsigned *tasks_create
(const double *h, int nclasses, unsigned ntasks, int kernel)
{
	int k;
	unsigned *tasks;
	const unsigned FACTOR = 10;
	
	tasks = smalloc(ntasks*sizeof(unsigned));
	memset(tasks, 0, ntasks*sizeof(unsigned));
	
	k = 0;
	for (int i = 0; i < nclasses; i++)
	{
		int n;
		
		n = floor(h[i]*ntasks);
		
		for (int j = 0; j < n; j++)
		{
			double x;
			
			x = (i + 1)*FACTOR;
		
			/* Check for corner cases. */
			if (x < 1)
				error("bad multiplying factor");
			
			switch (kernel)
			{
				/* Logarithmic kernel. */
				case KERNEL_LOGARITHMIC:
					x = x*log(x);
					break;
				
				/* Quadratic kernel. */
				case KERNEL_QUADRATIC:
					x = x*x;
					break;
				
				/* Linear kernel. */
				default:
				case KERNEL_LINEAR:
					break;
			}
			
			tasks[k++] = ceil(x);
		}
	}
	
	/* Fill up remainder tasks with minimum load. */
	for (unsigned i = k; i < ntasks; i++)
		tasks[i] = FACTOR;
	
	return (tasks);
}

/*============================================================================*
 *                                  SIMULATOR                                 *
 *============================================================================*/

/**
 * @brief Loop scheduler simulator.
 */
int main(int argc, const const char **argv)
{
	double *h;
	unsigned *tasks;
	readargs(argc, argv);

	h = histogram_create(args.pdf, args.nclasses, args.skewness);
	tasks = tasks_create(h, args.nclasses, args.ntasks, args.kernel);
	
	/* Print task classes. */
	for (int i = 0; i < args.nclasses; i++)
		fprintf(stderr, "%lf\n", h[i]);
		
	/* Print tasks. */
	for (int i = 0; i < args.ntasks; i++)
		printf("%u\n", tasks[i]);
	
	/* House keeping. */
	free(h);
	free(tasks);
}
