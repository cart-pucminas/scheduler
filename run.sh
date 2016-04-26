SMP=no

if [ smp == "yes" ]; then
	AFFINITY="0 2 4 6 8 10 12 14 16 18 20 22"
else
	AFFINITY="0 1 2 3 4 5 6 7 8 9 10 11"
fi

for seed in 1; do
	for strategy in dynamic srr; do 
			echo "== Running $strategy"
			for nthreads in 4; do
				LD_LIBRARY_PATH=libsrc/libgomp/libgomp/build/.libs/ \
				OMP_NUM_THREADS=$nthreads \
				GOMP_CPU_AFFINITY=$AFFINITY \
				OMP_SCHEDULE="pedro" \
				bin/is.$strategy timer.flag $seed \
				1> /dev/null
			done
	done
done
