SHELL=/bin/sh
BENCHMARK=is
BENCHMARKU=IS
CONTRIBDIR=$(CURDIR)/../../../../contrib

include ../config/make.def

include ../sys/make.common

OBJS = is.o \
       ${COMMON}/c_print_results.o \
       ${COMMON}/c_timers.o \
       ${COMMON}/c_wtime.o


${PROGRAM}: config ${OBJS}
	${CLINK} ${CLINKFLAGS} -fopenmp -o ${PROGRAM} ${OBJS} ../../../../contrib/lib/libgsl.a  ${C_LIB}

.c.o:
	${CCOMPILE} -I$(CONTRIBDIR)/include $<

is.o:             is.c  npbparams.h


clean:
	- rm -f *.o *~ mputil*
	- rm -f npbparams.h core
	- if [ -d rii_files ]; then rm -r rii_files; fi
