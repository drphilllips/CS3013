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
           
           int step; //d

} step_arg_t;

 

void* step_sum(void* step_args)
{
	//need to pass in start index, end index, cond var thread is waiting for	
	
	step_arg_t* arg = (step_arg_t*)step_args;
	int k = arg->beg;
	int d = arg->step;
	//printf("%d %d\n", arg->beg, arg->step);fflush(stdout);
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
	return 0;
}

 

int main(int argc, char* argv[])

{

    char* filename = argv[1];

           int n = atoi(argv[2]);

           int num_threads = atoi(argv[3]);

 

           if (n < 2 || num_threads < 1)

                      exit(EXIT_FAILURE);

 

           input = malloc(sizeof(int) * n);

           read_input_vector(filename, n, input);
           
           //auxiliary array, flip between arrays on each d step, then you don't need conditional variable to check if previous chunk has been updated yet before summing (Horn 2005)
          seq = (int*) malloc(sizeof(int) * n); 
           
          //pthread_t thread[num_threads];
           
          int* workl = (int*)malloc(sizeof(int));
          *workl = n/num_threads;
          
          step_arg_t* args = (step_arg_t*)malloc(sizeof(step_arg_t) * num_threads);
	  
	  int d;
          for(d = 0; d < log2(n); d += 1) //input when %2 == 0, seq when %2 == 1
          {
          	
			step_arg_t* argN = &args[0];
          		argN->end = *workl*0;//input index of end of chunk
          		//printf("%d\n", argN->end);fflush(stdout);
          		argN->beg = argN->end + *workl - 1;//input index of beginning of chunk
          		//printf("%d\n", argN->beg);fflush(stdout);
          		argN->step = d;
          		//printf("%d\n", argN->step);fflush(stdout);
         		
          		pthread_t tid1;
          		pthread_create(&tid1, NULL, step_sum, (void*)argN);
          		
			step_arg_t* arg2 = &args[1];
          		arg2->end = *workl*1;//input index of end of chunk
          		//printf("%d\n", argN->end);fflush(stdout);
          		arg2->beg = arg2->end + *workl - 1;//input index of beginning of chunk
          		//printf("%d\n", argN->beg);fflush(stdout);
          		arg2->step = d;
          		//printf("%d\n", argN->step);fflush(stdout);
         		
          		pthread_t tid2;
          		pthread_create(&tid2, NULL, step_sum, (void*)arg2);          		
          		
           		pthread_join(tid1, NULL);         		
          		pthread_join(tid2, NULL);
          	//barrier, wait for all threads to complete
          	
          
		  	//for(int k = n-1; k>0; k--) //index of input array, by starting at highest index, we avoid adding array value that has already been updated
		  	//{
			 // 	if(k >= (1u << d))
			 // 	{
			 // 		input[k] = input[(k) - (1u << d)] + input[k]; //critical section
			//  	}
			//} 
          } 
          
         
	if(d %2 == 0)
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
