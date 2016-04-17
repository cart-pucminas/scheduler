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

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <omp.h>

/*============================================================================*
 *                                   CLASSES                                  *
 *============================================================================*/

/******************/
/* Default Class  */
/******************/
#ifndef CLASS
#define CLASS 'C'
#endif

/*************/
/*  CLASS S  */
/*************/
#if CLASS == 'S'
#define  TOTAL_KEYS_LOG_2    16
#define  MAX_KEY_LOG_2       11
#define  NUM_BUCKETS_LOG_2   3
#endif

/*************/
/*  CLASS W  */
/*************/
#if CLASS == 'W'
#define  TOTAL_KEYS_LOG_2    20
#define  MAX_KEY_LOG_2       16
#define  NUM_BUCKETS_LOG_2   3
#endif

/*************/
/*  CLASS A  */
/*************/
#if CLASS == 'A'
#define  TOTAL_KEYS_LOG_2    23
#define  MAX_KEY_LOG_2       19
#define  NUM_BUCKETS_LOG_2   3
#endif

/*************/
/*  CLASS B  */
/*************/
#if CLASS == 'B'
#define  TOTAL_KEYS_LOG_2    25
#define  MAX_KEY_LOG_2       21
#define  NUM_BUCKETS_LOG_2   3
#endif

/*************/
/*  CLASS C  */
/*************/
#if CLASS == 'C'
#define  TOTAL_KEYS_LOG_2    27
#define  MAX_KEY_LOG_2       23
#define  NUM_BUCKETS_LOG_2   3
#endif

/*************/
/*  CLASS D  */
/*************/
#if CLASS == 'D'
#define  TOTAL_KEYS_LOG_2    31
#define  MAX_KEY_LOG_2       27
#define  NUM_BUCKETS_LOG_2   3
#endif


/**************************/
/*  Kernel Configuration  */
/**************************/
#if CLASS == 'D'
#define  TOTAL_KEYS          (1L << TOTAL_KEYS_LOG_2)
typedef  long INT_TYPE;
#else
#define  TOTAL_KEYS          (1 << TOTAL_KEYS_LOG_2)
typedef  int  INT_TYPE;
#endif
#define  MAX_KEY             (1 << MAX_KEY_LOG_2)
#define  NUM_BUCKETS         (1 << NUM_BUCKETS_LOG_2)
#define  NUM_KEYS            TOTAL_KEYS
#define  SIZE_OF_BUFFERS     NUM_KEYS  
#define  MAX_ITERATIONS      10

/*============================================================================*
 *                                  VARIABLES                                 *
 *============================================================================*/

/************************************/
/* These are the three main arrays. */
/* See SIZE_OF_BUFFERS def above    */
/************************************/
INT_TYPE key_array[SIZE_OF_BUFFERS],    
         key_buff1[MAX_KEY],
         key_buff2[SIZE_OF_BUFFERS],
         **key_buff1_aptr = NULL;

INT_TYPE **bucket_size, 
         bucket_ptrs[NUM_BUCKETS];
#pragma omp threadprivate(bucket_ptrs)

/**
 * @brief Random number generator.
 */
gsl_rng *r;

/*============================================================================*
 *                                    TIMER                                   *
 *============================================================================*/

static double start[3];
static double elapsed[3];

static void timer_clear(int n)
{
    elapsed[n] = 0.0;
}

void timer_start(int n)
{
	start[n] = omp_get_wtime();
}

void timer_stop(int n)
{
	double t, now;

	now = omp_get_wtime();
	t = now - start[n];
	elapsed[n] += t;
}

double timer_read(int n)
{
    return (elapsed[n]);
}

/*============================================================================*
 *                                  ALLOC_KEY_BUFF                            *
 *============================================================================*/

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
    int num_procs;

    num_procs = omp_get_max_threads();

    bucket_size = (INT_TYPE **)alloc_mem(sizeof(INT_TYPE *) * num_procs);

    for (i = 0; i < num_procs; i++)
        bucket_size[i] = (INT_TYPE *)alloc_mem(sizeof(INT_TYPE) * NUM_BUCKETS);

    #pragma omp parallel for
    for( i=0; i<NUM_KEYS; i++ )
        key_buff2[i] = 0;
}



/*============================================================================*
 *                                     RANK                                   *
 *============================================================================*/


void rank( int iteration )
{

    INT_TYPE    i, k;
    INT_TYPE    *key_buff_ptr, *key_buff_ptr2;

    int shift = MAX_KEY_LOG_2 - NUM_BUCKETS_LOG_2;
    INT_TYPE num_bucket_keys = (1L << shift);

    key_array[iteration] = iteration;
    key_array[iteration+MAX_ITERATIONS] = MAX_KEY - iteration;

/*  Setup pointers to key buffers  */
    key_buff_ptr2 = key_buff2;
    key_buff_ptr = key_buff1;

#pragma omp parallel private(i, k)
  {
    INT_TYPE *work_buff, m, k1, k2;
    int myid = 0, num_procs = 1;

    myid = omp_get_thread_num();
    num_procs = omp_get_num_threads();

    work_buff = bucket_size[myid];

/*  Initialize */
    for( i=0; i<NUM_BUCKETS; i++ )  
        work_buff[i] = 0;

/*  Determine the number of keys in each bucket */
    #pragma omp for schedule(static)
    for( i=0; i<NUM_KEYS; i++ )
        work_buff[key_array[i] >> shift]++;

/*  Accumulative bucket sizes are the bucket pointers.
    These are global sizes accumulated upon to each bucket */
    bucket_ptrs[0] = 0;
    for( k=0; k< myid; k++ )  
        bucket_ptrs[0] += bucket_size[k][0];

    for( i=1; i< NUM_BUCKETS; i++ ) { 
        bucket_ptrs[i] = bucket_ptrs[i-1];
        for( k=0; k< myid; k++ )
            bucket_ptrs[i] += bucket_size[k][i];
        for( k=myid; k< num_procs; k++ )
            bucket_ptrs[i] += bucket_size[k][i-1];
    }

/*  Sort into appropriate bucket */
    #pragma omp for schedule(static)
    for( i=0; i<NUM_KEYS; i++ )  
    {
        k = key_array[i];
        key_buff2[bucket_ptrs[k >> shift]++] = k;
    }

/*  The bucket pointers now point to the final accumulated sizes */
    if (myid < num_procs-1) {
        for( i=0; i< NUM_BUCKETS; i++ )
            for( k=myid+1; k< num_procs; k++ )
                bucket_ptrs[i] += bucket_size[k][i];
    }

	#pragma omp master
	timer_start(2);

/*  Now, buckets are sorted.  We only need to sort keys inside
    each bucket, which can be done in parallel.  Because the distribution
    of the number of keys in the buckets is Gaussian, the use of
    a dynamic schedule should improve load balance, thus, performance     */
#if defined(_SCHEDULE_STATIC_)
    #pragma omp for schedule(static, 1)
#elif defined(_SCHEDULE_DYNAMIC_)
    #pragma omp for schedule(dynamic)
#else
	#error "bad scheduler"
#endif
    for( i=0; i< NUM_BUCKETS; i++ ) {

/*  Clear the work array section associated with each bucket */
        k1 = i * num_bucket_keys;
        k2 = k1 + num_bucket_keys;
        for ( k = k1; k < k2; k++ )
            key_buff_ptr[k] = 0;

/*  Ranking of all keys occurs in this section:                 */

/*  In this section, the keys themselves are used as their 
    own indexes to determine how many of each there are: their
    individual population                                       */
        m = (i > 0)? bucket_ptrs[i-1] : 0;
        for ( k = m; k < bucket_ptrs[i]; k++ )
            key_buff_ptr[key_buff_ptr2[k]]++;  /* Now they have individual key   */
                                       /* population                     */

/*  To obtain ranks of each key, successively add the individual key
    population, not forgetting to add m, the total of lesser keys,
    to the first key population                                          */
        key_buff_ptr[k1] += m;
        for ( k = k1+1; k < k2; k++ )
            key_buff_ptr[k] += key_buff_ptr[k-1];

    }

	#pragma omp master
	timer_stop(2);

  } /*omp parallel*/
}

/*============================================================================*
 *                                  CREATE_SEQ                                *
 *============================================================================*/

void create_seq(void)
{
	int i;
	
	for (i = 0; i< TOTAL_KEYS; i++)
	{
		double num;
		
		do
			num = gsl_ran_exponential(r, 0.5);
		while (num > 2.0);

		key_array[i] = (INT_TYPE)((num/2.0)*MAX_KEY);
	}
}

/*============================================================================*
 *                                     MAIN                                   *
 *============================================================================*/

int main(int argc, char **argv)
{
	int i;
	const gsl_rng_type *T;
	
	((void)argc);
	((void)argv);
	
	/* Setup random number generator. */
	gsl_rng_env_setup();
	T = gsl_rng_default;
	r = gsl_rng_alloc(T);
	
	timer_clear(0);
	timer_clear(1);
	timer_clear(2);
	
timer_start(0);
	
	create_seq();
	alloc_key_buff();
	
timer_stop(0);
	
	/*
	 * Do one iteration to touch all
	 * data and code pages.
	 */
	rank(1);
	
timer_start(1);
	
	for (i = 1; i <= MAX_ITERATIONS; i++)
		rank(i);
	
timer_stop(1);

	printf("initialization: %8.3f\n", timer_read(0));
	printf("sorting:        %8.3f\n", timer_read(2));
	printf("benchmarking:   %8.3f\n", timer_read(1));

	return (0);
}
