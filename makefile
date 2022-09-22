PARAMS=-Wall -fdiagnostics-color=always
UNAME := $(shell uname -s)

all:
	make base
	make openmp
	make pthread
	make fork

base:
	gcc mandelbrot5.c -lm -o mb5.out $(PARAMS)
openmp:
ifeq ($(UNAME), Linux)
	gcc mandelbrot5_omp.c  -fopenmp -lm -o mb5_omp.out $(PARAMS)
endif
ifeq ($(UNAME), Darwin)
	gcc mandelbrot5_omp.c -Xclang -fopenmp -lomp -o mb5_omp.out $(PARAMS)
endif
pthread:
	gcc mandelbrot5_pthr.c -lm -pthread -o mb5_pthr.out $(PARAMS)

fork:
	gcc mandelbrot5_fork.c -lm -o mb5_fork.out $(PARAMS)
clean:
	rm *.o
	rm *.out