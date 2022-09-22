PARAMS=-Wall -fdiagnostics-color=always -lm
UNAME := $(shell uname -s)
OBJ=obj
all:
	make mb5
	make mb5_omp
	make mb5_pthr
	make mb5_fork


mb5: mandelbrot5.c
	gcc -c -o $@ $<

mb5_omp: mandelbrot5_omp.c
ifeq ($(UNAME), Linux)
	gcc -c -o $@ $<  -fopenmp $(PARAMS)
endif
ifeq ($(UNAME), Darwin)
	gcc -c -o $@ $< -Xclang -fopenmp -lomp $(PARAMS)
endif
mb5_pthr: mandelbrot5_pthr.c
	gcc -c -o $@ $< -pthread $(PARAMS)

mb5_fork: mandelbrot5_fork.c 
	gcc -c -o $@ $< $(PARAMS)

clean:
	rm *.o
	rm *.out