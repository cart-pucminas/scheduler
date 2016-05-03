SMP="no"

if [ smp == "yes" ]; then
	AFFINITY="0 2 4 6 8 10 12 14 16 18 20 22 24 26 28 30"
else
	AFFINITY="0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15"
fi

for seed in 1; do
	for strategy in static guided dynamic srr; do 
			echo "== Running $strategy"
			for nthreads in {2..16}; do
				LD_LIBRARY_PATH=libsrc/libgomp/libgomp/build/.libs/ \
				OMP_NUM_THREADS=$nthreads                           \
				GOMP_CPU_AFFINITY=$AFFINITY                         \
				OMP_SCHEDULE="pedro"                                \
				bin/is.$strategy timer.flag $seed                   \
				1> /dev/null                                        \
				2> is-normal-$strategy-$nthreads.out
			done
	done
done
