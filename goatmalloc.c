#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include "goatmalloc.h"

void* _arena_start = NULL;
void* _arena_end = NULL;
size_t a_size = -1;
int _initialized = 0;
node_t* head;
int statusno = 0;

extern int init(size_t size)
{
	if((int)size <= 0)
	{
		//perror("attempted size is negative\n");
		return ERR_BAD_ARGUMENTS;
	}
	else
	{
		printf("...requested size %d bytes\n", (int)size);fflush(stdout);
		int p_size = getpagesize();
		printf("...pagesize is %d bytes\n", (int)p_size);fflush(stdout);
		if(size < p_size) size = p_size;
		else if(size > p_size)
		{
			int diff = size%p_size;
			size += p_size - diff;
		}
		printf("...adjusting size with page boundaries\n...adjusted size is %d bytes\n", (int)size);fflush(stdout);
		int fd = open("/dev/zero", O_RDWR);
		if(fd == -1)
		{
			perror("file init failed\n");
			return ERR_SYSCALL_FAILED;
		}
		_arena_start = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
		if(_arena_start == MAP_FAILED)
		{
			perror("map init failed\n");
			return ERR_SYSCALL_FAILED;
		}
		printf("...mapping arena with mmap\narena starts at %p\n", _arena_start);fflush(stdout);
		printf("...arena ends at %p\n", _arena_start + size);fflush(stdout);
		_arena_end = _arena_start + size;
		_initialized = 1;
		
		//initialize chunk list
		head = (node_t*)_arena_start;
		head->size = size - sizeof(node_t);
		head->is_free = 1;
		head->fwd = NULL;
		head->bwd = NULL;
		printf("... initializing chunk list, header is at %p, free space starts at %p\n", head, (void*)head + sizeof(node_t));fflush(stdout);
	}
	return size;
}

extern int destroy()
{
	printf("...unmapping arena with munmap()\n");fflush(stdout);
	size_t length =  _arena_end - _arena_start;
	if(_initialized == 0)
	{
		//perror("arena unitialized\n");
		return ERR_UNINITIALIZED;
	}
	else
	{
		munmap(_arena_start, length);
		_arena_start = 0;
		_arena_end = 0;
		_initialized = 0;
		return 0;
	}
}

extern void* walloc(size_t size)
{
	void* buf;//point to beginning of allocated space
	if(_initialized == 0)
	{
		statusno = ERR_UNINITIALIZED;
		return NULL;
	}
	//find chunk
	node_t* tmp = head;//TODO head is NULL, need to debug
	printf("...tmp is first chunk at %p\n", tmp);fflush(stdout);
	printf("tmp size = %d\n", (int)tmp->size);fflush(stdout);
	printf("search size = %d\n", (int)size);fflush(stdout);
	int found = 0;
	while(!found && tmp != NULL)
	{
		if(tmp->size >= size && tmp->is_free == 1)
		{
			found = 1;
		}
		else tmp = tmp->fwd;
	}
	//tmp will point to free chunk if available
	if(tmp == NULL) //reached end of arena, no free chunk big enough
	{
		printf("..tmp is null\n");fflush(stdout);
		statusno = ERR_OUT_OF_MEMORY;
		return NULL;
	}
	else
	{
		//have found free chunk big enough for allocation + header
		buf = tmp + 1;//buff points to free mem
		printf("...beginning of free space is at %p\n", buf);fflush(stdout);
		tmp->size = size;
		printf("...new size is %d bytes\n", (int)tmp->size);fflush(stdout);
		tmp->is_free = 0;
		tmp->fwd = buf + size;
		node_t* nxt = tmp->fwd;
		printf("...next chunk is at %p\n", nxt);fflush(stdout);	
	}
	return buf;
}

void wfree(void* ptr)
{
	printf("...given ptr %p\n", ptr);fflush(stdout);
	node_t* header = ptr - sizeof(node_t);
	printf("...header begins at %p\n", header);fflush(stdout);
	header->is_free = 1;
	
}







