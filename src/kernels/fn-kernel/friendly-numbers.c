/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * x86/friendly-numbers.c - friendly_numbers() implementation.
 */

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#include <mylib/util.h>

extern int verbose;
extern int nthreads;

/*
 * Computes the Greatest Common Divisor of two numbers.
 */
static int gcd(int a, int b)
{
  int c;
  
  /* Compute greatest common divisor. */
  while (a != 0)
  {
     c = a;
     a = b%a;
     b = c;
  }
  
  return (b);
}

/*
 * Some of divisors.
 */
static int sumdiv(int n)
{
	int sum;    /* Sum of divisors. */
	int factor; /* Working factor.  */
	
	sum = 1 + n;
	
	/* Compute sum of divisors. */
	for (factor = 2; factor < n; factor++)
	{
		/* Divisor found. */
		if ((n%factor) == 0)
			sum += factor;
	}
	
	return (sum);
}

#if defined(_STATIC_SCHEDULE_)

/*
 * Computes friendly numbers.
 */
int friendly_numbers(int start, int end) 
{
	int n;        /* Divisor.                    */
	int *num;     /* Numerator.                  */
	int *den;     /* Denominator.                */
	int range;    /* Range of numbers.           */
	int i, j;     /* Loop indexes.               */
	int nfriends; /* Number of friendly numbers. */

	nfriends = 0;
	range = end - start + 1;
	
	num = smalloc(sizeof(int)*range);
	den = smalloc(sizeof(int)*range);
	
	#pragma omp parallel private(i, j, n)
	{
		/* Compute abundances. */
		#pragma omp for schedule(static)
		for (i = start; i <= end; i++) 
		{	
			j = i - start;
				
			num[j] = sumdiv(i);
			den[j] = i;
				
			n = gcd(num[j], den[j]);
			num[j] /= n;
			den[j] /= n;
		}
		
		#pragma omp for reduction(+:nfriends)
		for (i = 1; i < range; i++)
		{
			for (j = 0; j < i; j++)
			{
				/* Friends. */
				if ((num[i] == num[j]) && (den[i] == den[j]))
					nfriends++;
			}	
		}
	}

	free(num);
	free(den);
	
	return (nfriends);
}

#elif defined(_DYNAMIC_SCHEDULE_)

/*
 * Computes friendly numbers.
 */
int friendly_numbers(int start, int end) 
{
	int n;        /* Divisor.                    */
	int *num;     /* Numerator.                  */
	int *den;     /* Denominator.                */
	int range;    /* Range of numbers.           */
	int i, j;     /* Loop indexes.               */
	int nfriends; /* Number of friendly numbers. */

	nfriends = 0;
	range = end - start + 1;
	
	num = smalloc(sizeof(int)*range);
	den = smalloc(sizeof(int)*range);
	
	#pragma omp parallel private(i, j, n)
	{
		/* Compute abundances. */
		#pragma omp for schedule(dynamic)
		for (i = start; i <= end; i++) 
		{	
			j = i - start;
				
			num[j] = sumdiv(i);
			den[j] = i;
				
			n = gcd(num[j], den[j]);
			num[j] /= n;
			den[j] /= n;
		}
		
		#pragma omp for reduction(+:nfriends)
		for (i = 1; i < range; i++)
		{
			for (j = 0; j < i; j++)
			{
				/* Friends. */
				if ((num[i] == num[j]) && (den[i] == den[j]))
					nfriends++;
			}	
		}
	}

	free(num);
	free(den);
	
	return (nfriends);
}

#elif defined(_RUNTIME_SCHEDULE_)

/*
 * For now, libgomp hopes that we will
 * fill these structures. A better
 * way to achieve the same think would
 * be to do something like:
 * 
 *   #pragma omp paralell for tasks(myarray, ntasks)
 */
unsigned *__tasks;
unsigned __ntasks;

/*
 * Computes friendly numbers.
 */
int friendly_numbers(int start, int end) 
{
	int n;        /* Divisor.                    */
	int *num;     /* Numerator.                  */
	int *den;     /* Denominator.                */
	int range;    /* Range of numbers.           */
	int i, j;     /* Loop indexes.               */
	int nfriends; /* Number of friendly numbers. */

	nfriends = 0;
	range = end - start + 1;
	
	num = smalloc(sizeof(int)*range);
	den = smalloc(sizeof(int)*range);
	__tasks = smalloc(sizeof(unsigned)*range);
	__ntasks = range;
	
	/* Initialize tasks. */
	for (i = start; i < end; i++) {
		__tasks[i - start] = i;
		
		if (verbose){
			fprintf(stderr, "%u\n", __tasks[i - start]);
		}
	}
	
	#pragma omp parallel private(i, j, n)
	{
		/* Compute abundances. */
		#pragma omp for schedule(runtime)
		for (i = start; i <= end; i++) 
		{	
			j = i - start;
				
			num[j] = sumdiv(i);
			den[j] = i;
				
			n = gcd(num[j], den[j]);
			num[j] /= n;
			den[j] /= n;
		}
		
		#pragma omp for reduction(+:nfriends)
		for (i = 1; i < range; i++)
		{
			for (j = 0; j < i; j++)
			{
				/* Friends. */
				if ((num[i] == num[j]) && (den[i] == den[j]))
					nfriends++;
			}	
		}
	}

	free(num);
	free(den);
	free(__tasks);
	
	return (nfriends);
}

#endif

