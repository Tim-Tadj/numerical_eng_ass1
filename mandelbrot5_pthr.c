// Mandelbrot 5
// Written by Stephen So
// This is the fifth version of mandelbrot, rewritten from scratch, to work with gnuplot
// Added carray
// Histogram colour algorithm
// Single-threaded version

// Example coordinates: mandelbrot5 10000 -0.668 0.32 0.02

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <time.h>
#include <pthread.h>

#define NUM_THREADS 8
#define MAX_THREADS 64


#define WIDTH 1000	
#define HEIGHT 1000	

typedef struct {
	double xMin;  // minimum real value
	double xMax;  // maximum real value
	double yMin;  // minimum imaginary value
	double yMax;  // maximum imaginary value
	double step;  // step size in real/imaginary
	int height;
	int width;
	int *histogram;  // array for storing histogram values for colouring, dim: maxIter
	int *iterations;  // array for storing number of iterations, dim: WIDTH * HEIGHT
	double *pixels; // array for storing colour value of pixel, dim: WIDTH * HEIGHT
	double complex *carray; // array for storing complex numbers c, dim: WIDTH * HEIGHT
	int maxIter;  // maximum iterations before confident point is in mandelbrot set

	// added for pthreads
	int numThreads;
	pthread_t threads[MAX_THREADS];

} Parameters;


/* function prototypes */
void initialise(Parameters *);
void writeToFile(Parameters);
void histogramColouring(Parameters *);
void freeMemory(Parameters );
void mandelcompute_pthread(Parameters *);
void* mandelComputeThread(void *);


/* main program – execution begins here */
int main(int argc, char *argv[])
{
	int maxIter;
	double xc, yc, size;
	Parameters p;
	int numThreads = NUM_THREADS;
	// p.numThreads = NUM_THREADS;

	
	if (argc < 2) {
		// printf("Usage: mandelbrot maxIter [x y size]\n\nUsing default values\n");
		maxIter = 5000;
		p.xMin = p.yMin = -2;
		p.xMax = p.yMax = 2;
	}
	else if (argc == 2) {
		sscanf(argv[1], "%i", &maxIter);
		p.xMin = p.yMin = -2;
		p.xMax = p.yMax = 2;
	}
	else if (argc == 5) {
		sscanf(argv[1], "%i", &maxIter);
		sscanf(argv[2], "%lf", &xc);
		sscanf(argv[3], "%lf", &yc);
		sscanf(argv[4], "%lf", &size);
		
		size = size / 2;
		p.xMin = xc - size;
		p.yMin = yc - size;
		p.xMax = xc + size;
		p.yMax = yc + size;
	}
	else if (argc == 6) {
		sscanf(argv[1], "%i", &maxIter);
		sscanf(argv[2], "%lf", &xc);
		sscanf(argv[3], "%lf", &yc);
		sscanf(argv[4], "%lf", &size);
		sscanf(argv[5], "%i", &numThreads);

		size = size / 2;
		p.xMin = xc - size;
		p.yMin = yc - size;
		p.xMax = xc + size;
		p.yMax = yc + size;
	}

	p.maxIter = maxIter;
	p.width = WIDTH;
	p.height = HEIGHT;
	p.numThreads = numThreads;

	// printf("xMin = %lf\nxMax = %lf\nyMin = %lf\nyMax = %lf\nMaximum iterations = %i\n", p.xMin, p.xMax, p.yMin, p.yMax, p.maxIter);
	
	//time each function call using timespec
	struct timespec start_time, finish_time;
	double elapsed;
	
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	initialise(&p);
	clock_gettime(CLOCK_MONOTONIC, &finish_time);
	elapsed = (finish_time.tv_sec - start_time.tv_sec);
	elapsed += (finish_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;
	// printf("Initialisation time: %f seconds\n", elapsed);

	clock_gettime(CLOCK_MONOTONIC, &start_time);
	mandelcompute_pthread(&p);
	clock_gettime(CLOCK_MONOTONIC, &finish_time);
	elapsed = (finish_time.tv_sec - start_time.tv_sec);
	elapsed += (finish_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;
	printf("Pthr: threads:%d time:%f seconds\n", p.numThreads, elapsed);

	clock_gettime(CLOCK_MONOTONIC, &start_time);
	histogramColouring(&p);
	clock_gettime(CLOCK_MONOTONIC, &finish_time);
	elapsed = (finish_time.tv_sec - start_time.tv_sec);
	elapsed += (finish_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;
	// printf("Histogram colouring time: %f seconds\n", elapsed);

	clock_gettime(CLOCK_MONOTONIC, &start_time);
	writeToFile(p);
	clock_gettime(CLOCK_MONOTONIC, &finish_time);
	elapsed = (finish_time.tv_sec - start_time.tv_sec);
	elapsed += (finish_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;
	// printf("Writing to file time: %f seconds\n", elapsed);

	clock_gettime(CLOCK_MONOTONIC, &start_time);
	freeMemory(p);
	clock_gettime(CLOCK_MONOTONIC, &finish_time);
	elapsed = (finish_time.tv_sec - start_time.tv_sec);
	elapsed += (finish_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;
	// printf("Freeing memory time: %f seconds\n", elapsed);
	
	return (0);
}


void freeMemory(Parameters p)
{
	free(p.pixels);
	free(p.histogram);
	free(p.iterations);
	free(p.carray);
}

void writeToFile(Parameters p)
{
	FILE *fp = NULL;
	int i, j;
	double complex c;
	
	if ((fp = fopen("mandel.dat", "w")) == NULL) {
		perror("Cannot write mandel.dat file");
		exit(EXIT_FAILURE);
	}
	
	for (i = 0; i < p.height; i++) {
		for (j = 0; j < p.width; j++) {
			c = p.carray[i * p.width + j];
			fprintf(fp, "%.12lf %.12lf %.12lf\n", creal(c), cimag(c), p.pixels[i * p.width + j]);
		}
		fprintf(fp, "\n");
	}
	
	fclose(fp);
}

void histogramColouring(Parameters *p)
{
	int total, i, j, k, iter;
	
	// count the frequency of iterations
	for (i = 0; i < p->height; i++) {
		for (j = 0; j < p->width; j++) {
			iter = p->iterations[i * p->width + j];
			p->histogram[iter]++;
		}
	}

	// compute total of counts to normalise
	total = 0;
	for (i = 0; i < p->maxIter; i++) {
		total += p->histogram[i];
	}
	
	// compute pixel values using histogram algorithm
	for (i = 0; i < p->height; i++) {
		for (j = 0; j < p->width; j++) {
			if (p->iterations[i * p->width + j] == (p->maxIter - 1)) {
				p->pixels[i * p->width + j] = 0;  // if part of set, set to zero
			}
			else {
				for (k = 0; k < p->iterations[i * p->width + j]; k++) {
					p->pixels[i * p->width + j] += p->histogram[k] / (double)total;
				}
			}
		}
	}
}

void initialise(Parameters *p)
{
	int i, j;
	double x, y;

	p->step = (p->yMax - p->yMin) / p->width;
	
	if ((p->pixels = malloc(p->width * p->height * sizeof(double))) == NULL) {
		perror("Cannot allocate memory (pixels)");
		exit(EXIT_FAILURE);
	}
	
	if ((p->carray = malloc(p->width * p->height * sizeof(double complex))) == NULL) {
		perror("Cannot allocate memory (pixels)");
		exit(EXIT_FAILURE);
	}
	
	if ((p->iterations = malloc(p->width * p->height * sizeof(int))) == NULL) {
		perror("Cannot allocate memory (iterations)");
		exit(EXIT_FAILURE);
	}
	
	if ((p->histogram = malloc(p->maxIter * sizeof(int))) == NULL) {
		perror("Cannot allocate memory (histogram)");
		exit(EXIT_FAILURE);
	}
	
	// initialise array with zeros
	for (i = 0; i < p->height; i++) {
		for (j = 0; j < p->width; j++) {
			p->pixels[i * p->width + j] = 0.0;
		}
	}
	
	// initialise carray with real/imaginary values for c
	y = p->yMax;
	for (i = 0; i < p->height; i++) {
		x = p->xMin;
		for (j = 0; j < p->width; j++) {
			p->carray[i * p->width + j] = x + y * I;
			x += p->step;
		}
		y -= p->step;
	}
	
	for (i = 0; i < p->maxIter; i++) {
		p->histogram[i] = 0;
	}
}


void mandelcompute_pthread(Parameters *p)
{

	for (int i = 0; i < p->numThreads; i++) {
		pthread_create(&p->threads[i], NULL, mandelComputeThread, (void *)p);
	}

	
	for (int i = 0; i < p->numThreads; i++) {
		pthread_join(p->threads[i], NULL);
	}
}

void* mandelComputeThread(void *arg)
{
	Parameters *p = (Parameters *)arg;
	double absz;
	double complex c, z;
	int i, j, k;

	//get the thread id ising pthread_self()
	pthread_t thread_id = pthread_self();
	// get index where the thread id matches the thread id in the array
	int thread_idx = 0;
	for (int i = 0; i < p->numThreads; i++) {
		if (p->threads[i] == thread_id) {
			thread_idx = i;
			break;
		}
	}
	int block_size = p->height / p->numThreads;
	// use index to get the start and end of the block
	int i_start = block_size * thread_idx ;
	int i_end = i_start + block_size;
	//make sure the last thread does not go out of bounds
	if (thread_id == p->threads[p->numThreads - 1]) {
		//print thre
		i_end = p->height;
	}

	// printf("Thread %d: start = %d, end = %d\n", thread_idx, i_start, i_end);
	
	for (i = i_start; i < i_end; i++) {
		for (j = 0; j < p->width; j++) {
			c = p->carray[i * p->width + j];
			z = 0 + 0 * I;
		
			// test if z is part of the Mandelbrot set
			for (k = 0; k < p->maxIter; k++) {
				z = z * z + c;
				absz = cabs(z);
				
				if (absz >= 2.0)
					break;
			}
			
			if (k >= p->maxIter) {
				p->iterations[i * p->width + j] = p->maxIter - 1;
			}
			else {
				p->iterations[i * p->width + j] = k;
			}
		}
	}
	return NULL;
}
