for seed in {1..20}; do
	for strategy in dynamic srr; do 
			for nthreads in 8; do
				LD_LIBRARY_PATH=../../../libsrc/libgomp/libgomp/build/.libs/ \
				OMP_NUM_THREADS=$nthreads \
				GOMP_CPU_AFFINITY="0 2 4 6 8 10 12 14 16 18 20 22" \
				OMP_SCHEDULE="pedro" \
				bin/is.D.$strategy timer.flag $seed \
				1> /dev/null 
	#			2>> $strategy-$nthreads-$seed.out2
			done
	done
done
