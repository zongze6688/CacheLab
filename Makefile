#
# Student makefile for Cache Lab
#
# -fsigned-char makes char signed on aarch64 too, matching x86-64.
# trans.c and mul.c are compiled with -O0, as in the CMU version.
# solver.c (Bonus) is compiled with -O2 so that probe loops are not
# painfully slow, and with -fno-stack-protector so that the stack protector
# does not add symbols that test-solver's whitelist rejects.
#
CC = gcc
CFLAGS = -g -Wall -Werror -fsigned-char
SOLVER_CFLAGS = $(CFLAGS) -O2 -fno-stack-protector

all: csim test-csim test-trans tracegen test-mul tracegen-mul solver

csim: csim.c cachelab.c cachelab.h
	$(CC) $(CFLAGS) -o csim csim.c cachelab.c -lm

test-csim: test-csim.c
	$(CC) $(CFLAGS) -o test-csim test-csim.c

test-trans: test-trans.c perf.c perf.h trans.o cachelab.c cachelab.h
	$(CC) $(CFLAGS) -o test-trans test-trans.c perf.c cachelab.c trans.o

tracegen: tracegen.c trans.o cachelab.c cachelab.h
	$(CC) $(CFLAGS) -O0 -o tracegen tracegen.c trans.o cachelab.c

trans.o: trans.c cachelab.h
	$(CC) $(CFLAGS) -O0 -c trans.c

test-mul: test-mul.c perf.c perf.h mul.o cachelab.c cachelab.h
	$(CC) $(CFLAGS) -o test-mul test-mul.c perf.c cachelab.c mul.o

tracegen-mul: tracegen-mul.c mul.o cachelab.c cachelab.h
	$(CC) $(CFLAGS) -O0 -o tracegen-mul tracegen-mul.c mul.o cachelab.c

mul.o: mul.c cachelab.h
	$(CC) $(CFLAGS) -O0 -c mul.c

#
# Bonus: reverse-engineer a cache. ./solver is run by the provided
# test-solver, which supplies main() and the cache API.
#
solver.o: solver.c solver.h
	$(CC) $(SOLVER_CFLAGS) -c solver.c

solver: solver.o libsolve.c solver.h
	$(CC) $(SOLVER_CFLAGS) -o solver solver.o libsolve.c

#
# Clean the src directory (csim-ref and test-solver are provided, so they
# are kept)
#
clean:
	rm -f *.o *.tar
	rm -f csim test-csim test-trans tracegen test-mul tracegen-mul
	rm -f solver solver.out.*
	rm -f trace.all trace.f* trace.m* trace.tmp
	rm -f .csim_results .marker

.PHONY: all clean
