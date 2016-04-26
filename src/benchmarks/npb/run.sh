make clean
make all CLASS=B

for seed in 1; do
	for strategy in dynamic srr; do 
			echo "Running $strategy"
			for nthreads in 4; do
				LD_LIBRARY_PATH=../../../libsrc/libgomp/libgomp/build/.libs/ \
				OMP_NUM_THREADS=$nthreads \
				GOMP_CPU_AFFINITY="0 1 2 3" \
				OMP_SCHEDULE="pedro" \
				bin/is.B.$strategy timer.flag $seed \
				1> /dev/null 
	#			2>> $strategy-$nthreads-$seed.out2
			done
	done
done
