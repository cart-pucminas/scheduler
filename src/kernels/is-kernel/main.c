/*************************************************************************
 *                                                                       * 
 *       N  A  S     P A R A L L E L     B E N C H M A R K S  3.3        *
 *                                                                       *
 *                      O p e n M P     V E R S I O N                    *
 *                                                                       * 
 *                                  I S                                  * 
 *                                                                       * 
 ************************************************************************* 
 *                                                                       * 
 *   This benchmark is an OpenMP version of the NPB IS code.             *
 *   It is described in NAS Technical Report 99-011.                     *
 *                                                                       *
 *   Permission to use, copy, distribute and modify this software        *
 *   for any purpose with or without fee is hereby granted.  We          *
 *   request, however, that all derived work reference the NAS           *
 *   Parallel Benchmarks 3.3. This software is provided "as is"          *
 *   without express or implied warranty.                                *
 *                                                                       *
 *   Information on NPB 3.3, including the technical report, the         *
 *   original specifications, source code, results and information       *
 *   on how to submit new results, is available at:                      *
 *                                                                       *
 *          http://www.nas.nasa.gov/Software/NPB/                        *
 *                                                                       *
 *   Send comments or suggestions to  npb@nas.nasa.gov                   *
 *                                                                       *
 *         NAS Parallel Benchmarks Group                                 *
 *         NASA Ames Research Center                                     *
 *         Mail Stop: T27A-1                                             *
 *         Moffett Field, CA   94035-1000                                *
 *                                                                       *
 *         E-mail:  npb@nas.nasa.gov                                     *
 *         Fax:     (650) 604-3957                                       *
 *                                                                       *
 ************************************************************************* 
 *                                                                       * 
 *   Author: M. Yarrow                                                   * 
 *           H. Jin                                                      * 
 *                                                                       * 
 *************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include <omp.h>

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

union tick_t
{
  uint64_t tick;
  struct
  {
    uint32_t low;
    uint32_t high;
  } sub;
};

#define _GET_TICK(t)           \
	__asm__ volatile("rdtsc" : \
		"=a" ((t).sub.low),    \
		"=d" ((t).sub.high))

/*****************************************************************/
/* For serial IS, buckets are not really req'd to solve NPB1 IS  */
/* spec, but their use on some machines improves performance, on */
/* other machines the use of buckets compromises performance,    */
/* probably because it is extra computation which is not req'd.  */
/* (Note: Mechanism not understood, probably cache related)      */
/* Example:  SP2-66MhzWN:  50% speedup with buckets              */
/* Example:  SGI Indy5000: 50% slowdown with buckets             */
/* Example:  SGI O2000:   400% slowdown with buckets (Wow!)      */
/*****************************************************************/

/* This controls load imbalance. */
const unsigned NUM_BUCKETS_LOG_2 = 4;
const unsigned NUM_BUCKETS = (1 << 4);

#define CLASS_S 0
#define CLASS_W 1
#define CLASS_A 2
#define CLASS_B 3
#define CLASS_C 4
#define CLASS_D 5

/******************/
/* default values */
/******************/
#ifndef CLASS
#define CLASS CLASS_S
#endif

/*************/
/*  CLASS S  */
/*************/
#if (CLASS == CLASS_S)
#define  TOTAL_KEYS_LOG_2    16
#define  MAX_KEY_LOG_2       11
#endif


/*************/
/*  CLASS W  */
/*************/
#if CLASS == CLASS_W
#define  TOTAL_KEYS_LOG_2    20
#define  MAX_KEY_LOG_2       16
#endif

/*************/
/*  CLASS A  */
/*************/
#if CLASS == CLASS_A
#define  TOTAL_KEYS_LOG_2    23
#define  MAX_KEY_LOG_2       19
#endif


/*************/
/*  CLASS B  */
/*************/
#if CLASS == CLASS_B
#define  TOTAL_KEYS_LOG_2    25
#define  MAX_KEY_LOG_2       21
#endif


/*************/
/*  CLASS C  */
/*************/
#if CLASS == CLASS_C
#define  TOTAL_KEYS_LOG_2    27
#define  MAX_KEY_LOG_2       23
#endif


/*************/
/*  CLASS D  */
/*************/
#if CLASS == CLASS_D
#define  TOTAL_KEYS_LOG_2    31
#define  MAX_KEY_LOG_2       27
#endif


#if (CLASS == CLASS_D)
#define  TOTAL_KEYS          (1L << TOTAL_KEYS_LOG_2)
#else
#define  TOTAL_KEYS          (1 << TOTAL_KEYS_LOG_2)
#endif
#define  MAX_KEY             (1 << MAX_KEY_LOG_2)
#define  NUM_KEYS            TOTAL_KEYS
#define  SIZE_OF_BUFFERS     NUM_KEYS  
                                           

#define  MAX_ITERATIONS      10

	
typedef  uint64_t INT_TYPE;


/************************************/
/* These are the three main arrays. */
/* See SIZE_OF_BUFFERS def above    */
/************************************/
INT_TYPE *key_array,    
         *key_buff1,
         *key_buff2,
         **key_buff1_aptr = NULL;

INT_TYPE **bucket_size;

/*****************************************************************/
/*************      C  R  E  A  T  E  _  S  E  Q      ************/
/*****************************************************************/

void create_seq(void)
{
	gsl_rng * r;
	const gsl_rng_type * T;
	
	/* Setup random number generator. */
	gsl_rng_env_setup();
	T = gsl_rng_default;
	r = gsl_rng_alloc(T);
	
	for (INT_TYPE i = 0L; i < SIZE_OF_BUFFERS; i++)
	{
		double x;
		
		do
			x = gsl_ran_beta(r, 0.5, 0.5);
		while ((x == 0.0) || (x == 1.0));

		key_array[i] = ceil(x*MAX_KEY);
		if (key_array[i] >= MAX_KEY)
			key_array[i]--;
	}
	
	
	/* House keeping. */		
	gsl_rng_free(r);
}

/*****************************************************************/
/*****************    Allocate Working Buffer     ****************/
/*****************************************************************/

void *alloc_mem( size_t size )
{
    void *p;

    p = (void *)malloc(size);
    if (!p) {
        perror("Memory allocation error");
        exit(1);
    }
    return p;
}

void alloc_key_buff( void )
{
    INT_TYPE i;
    INT_TYPE num_procs;

    num_procs = omp_get_max_threads();

    bucket_size = (INT_TYPE **)alloc_mem(sizeof(INT_TYPE *) * num_procs);

    for (i = 0; i < num_procs; i++) {
        bucket_size[i] = (INT_TYPE *)alloc_mem(sizeof(INT_TYPE) * NUM_BUCKETS);
    }

    #pragma omp parallel for
    for( i=0; i<NUM_KEYS; i++ )
        key_buff2[i] = 0;
}

/*****************************************************************/
/*************             R  A  N  K             ****************/
/*****************************************************************/

#if defined(_SCHEDULE_SRR_)
extern void omp_set_workload(unsigned *, unsigned);
#endif

void rank(int iteration)
{

    INT_TYPE    i, k;
	union tick_t t0, t1;
    INT_TYPE    *key_buff_ptr, *key_buff_ptr2;
    INT_TYPE **bucket_ptrs;
    INT_TYPE num_procs;
    
    ((void) iteration);

    INT_TYPE shift = MAX_KEY_LOG_2 - NUM_BUCKETS_LOG_2;
    INT_TYPE num_bucket_keys = (1L << shift);
    
    num_procs = omp_get_max_threads();
    
    bucket_ptrs = alloc_mem(num_procs*sizeof(INT_TYPE *));
    for (i = 0; i < num_procs; i++)
		bucket_ptrs[i] = alloc_mem(NUM_BUCKETS*sizeof(INT_TYPE));
    
/*  Setup pointers to key buffers  */
	key_buff_ptr2 = key_buff2;
    key_buff_ptr = key_buff1;
    
    /* SRR stuff. */
	#if defined(_SCHEDULE_SRR_)
    unsigned *tasks;
    tasks=alloc_mem(NUM_BUCKETS*sizeof(unsigned));
    for (i = 0; i < NUM_BUCKETS; i++)
		tasks[i] = 0;
	
	for( i=0; i<NUM_KEYS; i++ )
		tasks[key_array[i] >> shift]++;
	if (iteration == 0)
	{
		for (i = 0; i < NUM_BUCKETS; i++)
			fprintf(stderr, "%u\n", tasks[i]);
	}
    omp_set_workload(tasks, NUM_BUCKETS);
	#endif


#pragma omp parallel private(i, k)
  {

    INT_TYPE *work_buff;
    INT_TYPE myid = 0;

    myid = omp_get_thread_num();

    work_buff = bucket_size[myid];

	/* Initialize. */
    for(i = 0; i < NUM_BUCKETS; i++)
		work_buff[i] = 0;        
		
	/* Determine the number of keys in each bucket. */
    #pragma omp for schedule(static)
    for( i=0; i<NUM_KEYS; i++)
		work_buff[key_array[i] >> shift]++;

/*  Accumulative bucket sizes are the bucket pointers.
    These are global sizes accumulated upon to each bucket */
    bucket_ptrs[myid][0] = 0;
    for( k=0; k< myid; k++ )  
        bucket_ptrs[myid][0] += bucket_size[k][0];

    for( i=1; i< NUM_BUCKETS; i++ ) { 
        bucket_ptrs[myid][i] = bucket_ptrs[myid][i-1];
        for( k=0; k< myid; k++ )
            bucket_ptrs[myid][i] += bucket_size[k][i];
        for( k=myid; k< num_procs; k++ )
            bucket_ptrs[myid][i] += bucket_size[k][i-1];
    }


/*  Sort into appropriate bucket */
    #pragma omp for schedule(static)
    for( i=0; i<NUM_KEYS; i++ )  
    {
        k = key_array[i];
        key_buff2[bucket_ptrs[myid][k >> shift]++] = k;
    }

/*  The bucket pointers now point to the final accumulated sizes */
    if (myid < num_procs-1) {
        for( i=0; i< NUM_BUCKETS; i++ )
            for( k=myid+1; k< num_procs; k++ )
                bucket_ptrs[myid][i] += bucket_size[k][i];
    }

  } /*omp parallel*/
  
	_GET_TICK(t0);
	
/*  Now, buckets are sorted.  We only need to sort keys inside
    each bucket, which can be done in parallel.  Because the distribution
    of the number of keys in the buckets is Gaussian, the use of
    a dynamic schedule should improve load balance, thus, performance     */

#if defined(_SCHEDULE_DYNAMIC_)
	#pragma omp parallel for schedule(dynamic)
#elif defined(_SCHEDULE_SRR_)
	#pragma omp parallel for schedule(runtime)
#else
	#pragma omp parallel for schedule(static, 1)
#endif
    for( i=0; i< NUM_BUCKETS; i++ ) {

    INT_TYPE myid = 0;

    myid = omp_get_thread_num();
    
    INT_TYPE m, k1, k2;
    
/*  Clear the work array section associated with each bucket */
        k1 = i * num_bucket_keys;
        k2 = k1 + num_bucket_keys;
        for ( k = k1; k < k2; k++ )
            key_buff_ptr[k] = 0;

/*  Ranking of all keys occurs in this section:                 */

/*  In this section, the keys themselves are used as their 
    own indexes to determine how many of each there are: their
    individual population                                       */
        m = (i > 0)? bucket_ptrs[myid][i-1] : 0;
        for ( k = m; k < bucket_ptrs[myid][i]; k++ )
            key_buff_ptr[key_buff_ptr2[k]]++;  /* Now they have individual key   */
                                       /* population                     */

/*  To obtain ranks of each key, successively add the individual key
    population, not forgetting to add m, the total of lesser keys,
    to the first key population                                          */
        key_buff_ptr[k1] += m;
        for ( k = k1+1; k < k2; k++ )
            key_buff_ptr[k] += key_buff_ptr[k-1];

    }
    
	_GET_TICK(t1);
  
  printf("%" PRIu64 "\n", t1.tick - t0.tick);


#if defined(_SCHEDULE_SRR_)
	free(tasks);
#endif
}      


/*****************************************************************/
/*************             M  A  I  N             ****************/
/*****************************************************************/

int main( int argc, char **argv )
{
	((void)argc);
	((void)argv);
	
	key_array = alloc_mem(SIZE_OF_BUFFERS*sizeof(INT_TYPE));
	key_buff1 = alloc_mem(MAX_KEY*sizeof(INT_TYPE));
	key_buff2 = alloc_mem(SIZE_OF_BUFFERS*sizeof(INT_TYPE));

    create_seq();

    alloc_key_buff();

/*  Do one interation for free (i.e., untimed) to guarantee initialization of  
    all data and code pages and respective tables */
    rank(0);	
    

/*  This is the main iteration */
    for (int i = 1; i <= MAX_ITERATIONS; i++)
		rank(i);
	
	/* House keeping. */
	free(key_array);
	free(key_buff1);
	free(key_buff2);

    return 0;
}
