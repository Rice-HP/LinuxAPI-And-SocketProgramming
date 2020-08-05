#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(){
	pid_t pid = 0;

	int i;
	for(i = 0; i < 5; i++)
	{
		pid = fork();
		if(pid == 0)
		{
			printf("%d new child pid:%d, parent:%d \n",i,getpid(),getppid());
			break;
		}
	}

	sleep(i);

	if(i < 5)
		printf("child_%d end, pid:%d \n",i,getpid());

	if(pid > 0)
	{
		printf("I am father:%d\n",getpid());
	}
	return 0;
}
