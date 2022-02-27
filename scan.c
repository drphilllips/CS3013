#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_SIZE 256

int* input;
int* seq;
sem_t sem;
int workl;
//int d = 0;
int thrds_arrived = 0;
int num_threads;
int n;
int log_size = 0;

sem_t phase1;
sem_t phase2;
sem_t lock;

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
           //int dst; // distance between "neighbors"
           int beg; // first operation
           int end; // final operation
           //int step; //d
} step_arg_t;

void* step_sum(void* step_args)
{
	//need to pass in start index, end index, cond var thread is waiting for	
	
	step_arg_t* arg = (step_arg_t*)step_args;
	int k = arg->beg;
	int d;
	//printf("%d %d %d\n", arg->beg, arg->end, n);fflush(stdout);
	for(d = 0; d<log2(n); d+=1)
	{
		while(k>=arg->end) //index of input array, by starting at highest index, we avoid adding array value that has already been updated
		{
			if(k >= (1u << d))
			{
				if(d%2 == 0)
					seq[k] = input[(k) - (1u << d)] + input[k]; //critical section
				else
					input[k] = seq[(k) - (1u << d)] + seq[k];
			}
			else
			{
				if(d%2 == 0)
					seq[k] = input[k]; //critical section
				else
					input[k] = seq[k];			
			}
			k--;
		} 
		k = arg->beg;
		//barrier, ensure threads wait for every thread to finish step before moving to next one
		sem_wait(&lock);
		thrds_arrived += 1;
		if(thrds_arrived == num_threads)
		{
			sem_wait(&phase2);
			sem_post(&phase1);
		}
		sem_post(&lock);
		
		sem_wait(&phase1);
		sem_post(&phase1);
		
		
		
		sem_wait(&lock);
		thrds_arrived -= 1;
		if(thrds_arrived == 0)
		{
			sem_wait(&phase1);
			sem_post(&phase2);
		}
		sem_post(&lock);
		
		sem_wait(&phase2);
		sem_post(&phase2);
		//printf("%d\n", d);fflush(stdout);
		
	} 
	log_size = d;
	return 0;
}

int main(int argc, char* argv[])

{
	sem_init(&phase1, 0, 0);
	sem_init(&phase2, 0, 1);
	sem_init(&lock, 0, 1);
	
    char* filename = argv[1];

           n = atoi(argv[2]);

           num_threads = atoi(argv[3]);

           if (n < 2 || num_threads < 1)
                      exit(EXIT_FAILURE);

           input = malloc(sizeof(int) * n);

           read_input_vector(filename, n, input);
           
           //auxiliary array, flip between arrays on each d step, then you don't need conditional variable to check if previous chunk has been updated yet before summing (Horn 2005)
          seq = (int*) malloc(sizeof(int) * n); 
          for(int i = 0; i<n; i++)
          {
          	seq[i] = 0;
          }
           
          pthread_t** thread = (pthread_t**)malloc(sizeof(pthread_t*) * num_threads);//array of thread pointers
           
          int* workl = (int*)malloc(sizeof(int));
          *workl = n/num_threads;
          
          step_arg_t** args = (step_arg_t**)malloc(sizeof(step_arg_t*) * num_threads);
          
	  int* log2 = (int*)malloc(sizeof(int));
          for(int i = 0; i<num_threads; i++)
          {
          	thread[i] = (pthread_t*)malloc(sizeof(pthread_t));
          	//initialize thread ptrs
          	args[i] = (step_arg_t*)malloc(sizeof(step_arg_t));
		step_arg_t* argN = args[i];
        	argN->end = *workl*i;//input index of end of chunk
        	//printf("%d\n", argN->end);fflush(stdout);
        	argN->beg = argN->end + *workl - 1;//input index of beginning of chunk
        	//printf("%d\n", argN->beg);fflush(stdout);
        	//argN->step = d;
        	//printf("%d\n", argN->step);fflush(stdout);
         		
        	pthread_create(thread[i], NULL, step_sum, (void*)argN);          		
        }
        for(int i = 0;i<num_threads; i++)
        {
        	pthread_join(*thread[i], (void**)&log2);
        }

          	//barrier, wait for all threads to complete
          	
          
		  	//for(int k = n-1; k>0; k--) //index of input array, by starting at highest index, we avoid adding array value that has already been updated
		  	//{
			 // 	if(k >= (1u << d))
			 // 	{
			 // 		input[k] = input[(k) - (1u << d)] + input[k]; //critical section
			//  	}
			//}  
          
         
	if(log_size %2 == 0)
	{
           for (int i=0; i < n; i++) 
           {

                      printf("%d\n", input[i]);

           } 
       } 
       else
       {
           for (int i=0; i < n; i++) 
           {

                      printf("%d\n", seq[i]);

           }       
       }

}
