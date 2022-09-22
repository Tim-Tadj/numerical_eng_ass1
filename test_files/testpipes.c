#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define MSG_LEN 11

#define IPC_DIM 2

enum {READ, WRITE};

int print_array(int *array, int size);
int chread(int fd, int *buf, int count, int chunk_size);
int chwrite(int fd, int *buf, int count, int chunk_size);



int main(void)
{
	int n_process = 5;
	int n_children = n_process-1;
	int fd[n_children][IPC_DIM];
	int mesg[MSG_LEN] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

	int sleep_array[4] = {2, 1, 3, 1};

	
	// create pipes
	for (int i = 0; i < n_children; i++)
	{
		if (pipe(fd[i]) == -1)
		{
			perror("pipe");
			exit(EXIT_FAILURE);
		}
	}
	//print pipe file descriptors
	printf("Pipe file descriptors: \n");
	for (int i = 0; i < n_children; i++)
	{
		printf("%d %d\n", fd[i][READ], fd[i][WRITE]);
	}

	pid_t child_id =0;

	int proc_index = 0;

	for (int i = 1; i < n_process; i++) {

		if ((child_id = fork()) == 0) {
			proc_index = i;
			break;
		}
	}
	if (child_id == 0) {
		printf("Child %d\n", proc_index);
		printf("I am the child\n");
		// close(fd[(proc_index-1)*IPC_DIM+READ]); // close the write end of pipe
		sleep(sleep_array[proc_index-1]);
		
		for (int i = 0; i < MSG_LEN; i++) {
			mesg[i] = mesg[i] * proc_index;
		}
		chwrite(fd[(proc_index-1)][WRITE], mesg, MSG_LEN, 4);
		exit(0);
	} else {
		printf("Parent\n");
		printf("I am the parent\n");
		//wait for all children to finish
		// for (int i = 0; i < n_children; i++) {
		// 	wait(NULL);
		// }
		// wait(NULL);
		for (int i = 0; i < n_children; i++) {

			int mesg2[MSG_LEN] = {0};
			// close(fd[i*IPC_DIM+WRITE]); // close the write end of pipe
			chread(fd[i][READ], mesg2, MSG_LEN, 4);
			printf("got msg from child %d\n", i+1);
			print_array(mesg2, MSG_LEN);
			// close(fd[i*IPC_DIM+READ]); // finish by closing the pipe
		}
	}
	
	return (0);
}


int print_array(int *array, int size) {
	for (int i = 0; i < size; i++) {
		printf("%d ", array[i]);
	}
	printf("\n");
	return 0;
}



int chread(int fd, int *buf, int total_ints, int chunk_size)
{
	//count is number of ints to read
	//chunk_size is number of ints to read at a time
	int bytes_read = 0;

	for (int i = 0; i < total_ints ; i += chunk_size) {
		bytes_read += read(fd, &buf[i], chunk_size*sizeof(int));
	}
	if (bytes_read != total_ints*sizeof(int)) {
		bytes_read += read(fd, &buf[bytes_read-chunk_size-1], total_ints*sizeof(int) - bytes_read);
	}
	return bytes_read;
}

int chwrite(int fd, int *buf, int total_ints, int chunk_size)
{
	//count is number of ints to write
	//chunk_size is number of ints to write at a time
	int bytes_written = 0;

	for (int i = 0; i < total_ints ; i += chunk_size) {
		bytes_written += write(fd, &buf[i], chunk_size*sizeof(int));
	}
	if (bytes_written != total_ints*sizeof(int)) {
		bytes_written += write(fd, &buf[bytes_written-chunk_size-1], total_ints*sizeof(int) - bytes_written);
	}
	return bytes_written;
}