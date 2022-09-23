PARAMS=-Wall -fdiagnostics-color=always -lm
UNAME := $(shell uname -s)
OBJ=obj
all:
	make mb5.out
	make mb5_omp.out
	make mb5_pthr.out
	make mb5_fork.out


mb5.out: mandelbrot5.c
	gcc $< -o $@ $(PARAMS)

mb5_omp.out: mandelbrot5_omp.c
ifeq ($(UNAME), Linux)
	gcc $< -o $@   -fopenmp $(PARAMS)
endif
ifeq ($(UNAME), Darwin)
	gcc $< -o $@ -Xclang -fopenmp -lomp $(PARAMS)
endif
mb5_pthr.out: mandelbrot5_pthr.c
	gcc $< -o $@  -pthread $(PARAMS)

mb5_fork.out: mandelbrot5_fork.c 
	gcc $< -o $@ $(PARAMS)

clean:
	rm *.out