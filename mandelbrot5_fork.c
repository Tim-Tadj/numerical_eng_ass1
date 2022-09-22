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
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>


#define WIDTH 1000	
#define HEIGHT 1000	


#define NUM_PROCESSES 8
#define MAX_PROCESSES 64

#define IPC_DIM 2

enum {READ, WRITE};
enum {CHILD, PARENT};


// enum {IPC_PIPE, IPC_SOCKET};

int IPC_TYPE;

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

	// added for fork
	int numProcesses;
	int process_idx;
	char ipctype; // 'p' for pipe, 's' for socket

} Parameters;


/* function prototypes */
void initialise(Parameters *);
void writeToFile(Parameters);
void histogramColouring(Parameters *);
void freeMemory(Parameters );
void mandelcompute_fork(Parameters *);
// void mandelcompute_fork_sockets(Parameters *);
void mandelComputeProcess(Parameters *);
void config_pipes(int *fd, int n_pipes);
void config_sockets(int *fd, int n_sockets);
int chread(int fd, int* buf, int count, int chunk_size);
int chwrite(int fd, int* buf, int count, int chunk_size);


/* main program – execution begins here */
int main(int argc, char *argv[])
{
	int maxIter;
	double xc, yc, size;
	char ipctype;
	Parameters p;
	p.numProcesses = NUM_PROCESSES;
	
	// IPC_TYPE = IPC_SOCKET;

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
		sscanf(argv[5], "%i", &p.numProcesses);
		
		size = size / 2;
		p.xMin = xc - size;
		p.yMin = yc - size;
		p.xMax = xc + size;
		p.yMax = yc + size;
	}
	else if (argc == 7) {
		sscanf(argv[1], "%i", &maxIter);
		sscanf(argv[2], "%lf", &xc);
		sscanf(argv[3], "%lf", &yc);
		sscanf(argv[4], "%lf", &size);
		sscanf(argv[5], "%i", &p.numProcesses);
		sscanf(argv[6], "%c", &ipctype);
		
		size = size / 2;
		p.xMin = xc - size;
		p.yMin = yc - size;
		p.xMax = xc + size;
		p.yMax = yc + size;
	}
	else {
		printf("Invalid num params\n");
		exit(1);
	}

	p.maxIter = maxIter;
	p.width = WIDTH;
	p.height = HEIGHT;
	p.ipctype = ipctype;


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
	mandelcompute_fork(&p);
	clock_gettime(CLOCK_MONOTONIC, &finish_time);
	elapsed = (finish_time.tv_sec - start_time.tv_sec);
	elapsed += (finish_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;
	printf("Fork: threads:%d time:%f_seconds ipc:%c\n", p.numProcesses, elapsed, p.ipctype);

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

// function to pipe the output array of ints to parent process 
int chread(int fd, int *buf, int total_ints, int chunk_size)
{
	//total_ints is number of ints to read
	//chunk_size is number of ints to read at a time
	int bytes_read = 0;

	for (int i = 0; i < total_ints ; i += chunk_size) {
		bytes_read += read(fd, &buf[i], chunk_size*sizeof(int));
		if (bytes_read <0) {
			perror("read");
			return -1;
		}
	}
	if (bytes_read != total_ints*sizeof(int)) {
		bytes_read += read(fd, &buf[bytes_read-chunk_size-1], total_ints*sizeof(int) - bytes_read);
		if (bytes_read <0) {
			perror("read");
			return -1;
		}
	}
	return bytes_read;
}

int chwrite(int fd, int *buf, int total_ints, int chunk_size)
{
	//total_ints is number of ints to write
	//chunk_size is number of ints to write at a time
	int bytes_written = 0;

	for (int i = 0; i < total_ints ; i += chunk_size) {
		bytes_written += write(fd, &buf[i], chunk_size*sizeof(int));
		if (bytes_written <0) {
			perror("write");
			return -1;
		}
	}
	if (bytes_written != total_ints*sizeof(int)) {
		bytes_written += write(fd, &buf[bytes_written-chunk_size-1], total_ints*sizeof(int) - bytes_written);
		if (bytes_written <0) {
			perror("write");
			return -1;
		}
	}
	return bytes_written;
}



void mandelcompute_fork(Parameters *p)
{
	int n_processes = p->numProcesses;
	int n_children = n_processes-1;
	int fd[n_children][IPC_DIM];

	// do pipes
	if (p->ipctype=='p'){
		// printf("Using pipes to compute Mandelbrot set\n");
		for (int i = 0; i < n_children; i++){
			if (pipe(fd[i]) < 0){
				perror("pipe creation failed");
				exit(EXIT_FAILURE);
			}
		}
	}
	else if (p->ipctype=='s'){
		// printf("Using sockets to compute Mandelbrot set\n");
		for (int i = 0; i < n_children; i++){
			if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd[i]) < 0){
				perror("socket creation failed");
				exit(EXIT_FAILURE);
			}
		}
	}
	else{
		printf("Invalid IPC type\n");
		exit(EXIT_FAILURE);
	}
	

	
	pid_t child_id;

	// pid_t child_pids[n_children];
	// pid_t parent_id = getpid();
	
	int process_idx = 0;
	p->process_idx = process_idx;
	int chunk_size = p->height / p->numProcesses;
	
	int i_start[p->numProcesses];
	int i_end[p->numProcesses];
	for (int i = 0; i < p->numProcesses; i++)
	{
		i_start[i] = i * chunk_size;
		i_end[i] = (i + 1) * chunk_size;
	}
	i_end[p->numProcesses - 1] = p->height;



	// create child processes and store their process ids
	// printf("Forking %d processes\n", p->numProcesses -1);
	for (int i = 1; i < n_processes; i++) {
		child_id = fork();
		if (child_id == 0) {
			process_idx = i;
			break;
		}
	}
	chunk_size = i_end[process_idx] - i_start[process_idx];

	if (child_id == 0) {
		p->process_idx = process_idx;
		mandelComputeProcess(p);
		// printf("child %d writing to fd %i\n", process_idx-1, fd[process_idx-1][WRITE]);
		int* buf = &p->iterations[i_start[process_idx]*p->width];
		if (p->ipctype=='p'){
			chwrite(fd[process_idx-1][WRITE], buf, chunk_size*p->width, 1024);
		}
		else if (p->ipctype=='s'){
			chwrite(fd[process_idx-1][CHILD], buf, chunk_size*p->width, 1024);
		}
		exit(0);
	} else {
		// printf("I am the parent\n");
		mandelComputeProcess(p);
		for (int i = 0; i < n_children; i++) {
			chunk_size = i_end[i+1] - i_start[i+1];
			int* buf = &p->iterations[i_start[i+1]*p->width];
			// printf("parent reading from fd %i\n", fd[i][READ]);
			if(p->ipctype=='p'){
				chread(fd[i][READ], buf, p->width*chunk_size, 1024);
			}
			else if(p->ipctype=='s'){
				chread(fd[i][PARENT], buf, p->width*chunk_size, 1024);
			}
		}
		// printf("parent finished reading from children\n");
	}

	// checkl for non-zero elements in iterations array
	int count = 0;
	for (int i = 0; i < p->height; i++) {
		for (int j = 0; j < p->width; j++) {
			if (p->iterations[i * p->width + j] != 0) {
				count++;
			}
		}
	}
	// printf("non-zero elements in iterations array: %d\n", count);
}
	



void mandelComputeProcess(Parameters *p)
{
	double absz;
	double complex c, z;
	int i, j, k, block_size, i_start, i_end;

	//calc block size from height and num processes
	block_size = p->height / p->numProcesses;

	// printf("Process %d: block size = %d\n", p->process_idx, block_size);
	// use index to get the start and end of the block
	i_start = block_size * p->process_idx;
	i_end = i_start + block_size;
	if (p->process_idx == p->numProcesses - 1) {
		i_end = p->height;
	}

	// printf("process %d doing work from %d to %d\n", p->process_idx, i_start, i_end);
	
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
	return;
}
