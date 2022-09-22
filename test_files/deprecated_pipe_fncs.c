void pipe_to_parent(Parameters *p, int c2p[2], int chunk_size, int position)
{
	int* temp_iterations = malloc(chunk_size * p->width * sizeof(int));
	for (int i = 0; i < chunk_size; i++) {
		for (int j = 0; j < p->width; j++) {
			temp_iterations[i * p->width + j] = p->iterations[(position + i) * p->width + j];
		}
	}
	
	close(c2p[0]);
	if(write(c2p[1], temp_iterations, chunk_size * p->width * sizeof(int)+1) == -1)
	{
		perror("write");
		exit(1);
	}
	
	//close the write end of the pipe
	close(c2p[1]);
}

void pipe_from_children(Parameters *p, int n_children, int* c2p, int* i_start, int* i_end)
{
	if (n_children>0)
	{
		// printf("parent reading from pipes\n");
		for (int i = 0; i < n_children; i++)
		{
			int chunk_size = i_end[i+1] - i_start[i+1];
			int* temp_iterations = malloc(chunk_size * p->width * sizeof(int));
			close(c2p[i*n_children + 1]);
			read(c2p[i*n_children + 0], temp_iterations, chunk_size * p->width * sizeof(int));
			for (int j = 0; j < chunk_size; j++) {
				for (int k = 0; k < p->width; k++) {
					p->iterations[(i_start[i+1] + j) * p->width + k] = temp_iterations[j * p->width + k];
				}
			}
			close(c2p[i*n_children + 0]);
		}
	}
}
