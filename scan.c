#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_SIZE 256

int chunk_size = 0;
int neighbor_distance = 1;
int* input;
int* copy;

void read_input_vector(const char* filename, int n, int* array)
{
  	FILE *fp;
  	char *line = malloc(MAX_LINE_SIZE+1);
  	size_t len = MAX_LINE_SIZE;
  	ssize_t read;

  	fp = strcmp(filename, "-") ? fopen(filename, "r") : stdin;

  	assert(fp != NULL && line != NULL);

  	int index = 0;

  	while ((read = getline(&line, &len, fp)) != -1)
  	{
    	array[index] = atoi(line);
    	index++;
  	}

  	free(line);
  	fclose(fp);
}

typedef struct __step_arg_t {
	int dst; // distance between "neighbors"
	int beg; // first operation
	int end; // final operation
} step_arg_t;

void* sum_neighbors(void* step_args)
{
	step_arg_t* m = (step_arg_t*) step_args;
    // add up values [neighbor_distance] to the left of each value, from start
    for (int op = m->beg; op <= m->end; op++) {
    	copy[op] += input[op - m->dst]; // data sharing w/ original arr, need to use a copy arr
    }
    return NULL;
}

int main(int argc, char* argv[])
{
    char* filename = argv[1];
  	int n = atoi(argv[2]);
  	int num_threads = atoi(argv[3]);

  	if (n < 2 || num_threads < 1)
  	  	exit(EXIT_FAILURE);

  	int *input= malloc(sizeof(int) * n);
  	read_input_vector(filename, n, input);
  	
  	int ops = n - 1; // total number of operations (at the start)
  	int step = 0; // current step
  	while (ops > 0) { // while loop for each step
  		int ops_per_thread = ops / num_threads;
  		int extra_ops = ops_per_thread % num_threads; // TODO: divide this evenly between threads
  		// make one copy of input arr for each step (avoid data sharing)
  		copy = (int*) malloc(sizeof(int) * n);
  		for (int i = 0; i < n; i++) {
  			copy[i] = input[i];
  		}
  		// initialize threads for each step
  		pthread_t* pthreads[num_threads];
  		
  		for (int thread = 0; thread < num_threads; thread++) { // for each thread
  		
  			pthread_t tid; 			// initialize this thread
  			pthreads[thread] = &tid;	// add to list of threads
  			// assemble arguments for this thread
  			step_arg_t args;
  			args.dst = 1u << step; // 2^n 
  			args.beg = ops_per_thread * thread + args.dst;
  			args.end = ops_per_thread * (thread + 1) + args.dst - 1;
  			if (thread == num_threads - 1)	// add all extra operations to final thread TODO: change
  				args.end += extra_ops;
  			// create thread
  			pthread_create(&tid, NULL, sum_neighbors, (void*) &args);
  		}
  		// join threads after each step
  		for (int thread = 0; thread < num_threads; thread++) {
  			pthread_join((pthread_t) pthreads[thread], NULL);
  		}
  		// TODO: fill input arr with copy arr data
  		for (int i = 0; i < n; i++) {
  			input[i] = copy[i];
  		}
  		
  		ops -= 1u << step; // ops -= 2^step# after every step
  		step++;
  	}
  
  	for (int i=0; i < n; i++) {
  	  	printf("%d\n", input[i]); 
  	}   
}


