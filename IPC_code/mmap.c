#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(){
	int* mem = mmap(NULL, 4, PROT_READ | PROT_WRITE, MAP_SHARED|MAP_ANON, -1, 0);
	if(mem == MAP_FAILED)
	{
		perror("mmap");
		return -1;
	}

	pid_t pid = fork();

	if(pid == 0){
		*mem = 666;
		printf("child mem:%d\n",*mem);
		sleep(3);
		printf("child mem:%d\n",*mem);
	}
	else
	{
		sleep(1);
		printf("parent mem:%d\n",*mem);
		*mem = 999;
		printf("parent mem:%d\n",*mem);
		wait(0);
	}

	munmap(mem,4);
	return 0;
}
