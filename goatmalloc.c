

#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "goatmalloc.h"

void* _arena_start = NULL;
size_t _arena_size = -1;
int _initialized = 0;

extern int init(size_t size) {

	_initialized = 0;

	printf("Initializing Arena:\n");fflush(stdout);
	
	if ((int) size <= 0) {
		return ERR_BAD_ARGUMENTS;
	}
	
	printf("...requested size %zu bytes\n", size);fflush(stdout);
	const int page_size = getpagesize(); // 4096
	printf("...page size is %d bytes\n", page_size);fflush(stdout);
	printf("...adjusting size with page boundaries\n");fflush(stdout);
	_arena_size = page_size;
	while (_arena_size < size)
		_arena_size += page_size;
	printf("...adjusted size is %zu bytes\n", _arena_size);fflush(stdout);
	printf("...mapping arena with mmap()\n");fflush(stdout);
	
	int fd = open("/dev/zero", O_RDWR);
	
	if (fd == -1) {
		perror("...failed in initialization, open");
		return ERR_SYSCALL_FAILED;
	}
	
	_arena_start = mmap(NULL, _arena_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	
	if (_arena_start == MAP_FAILED) {
		perror("...failed in initialization, mmap");
		return ERR_SYSCALL_FAILED;
	}
	
	printf("...arena starts at %p\n", _arena_start);fflush(stdout);
	printf("...arena ends at %p\n", _arena_start + _arena_size);fflush(stdout);
	printf("...initializing header for initial free chunk\n");fflush(stdout);
	int header_size = 32;
	// TODO: initialize headers
	
	printf("...header size is %d bytes\n", header_size);
	
	_initialized = 1;
	return _arena_size;
}

extern int destroy() {

	printf("Destroying Arena:\n");fflush(stdout);

	if (!_initialized) {
		return ERR_UNINITIALIZED;
	}

	printf("...unmapping arena with munmap()\n");fflush(stdout);
	if (_arena_start != NULL && _arena_size > 0) {
		munmap(_arena_start, _arena_size);
		_arena_start = NULL;
	}
	
	return 0;
}
/*
int main(int argc, char* argv[]) {
	init(-1);
	destroy();
	return 0;
}
*/


