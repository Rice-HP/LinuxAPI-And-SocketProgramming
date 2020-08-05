#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(){
	pid_t pid;

	pid = fork();

	if(pid < 0)
	{
		perror("fork erro");
	}
	else if(pid == 0){
		printf("I am child, pid:%d, ppid:%d\n",getpid(),getppid());
	}
	else if(pid > 0)
	{
		printf("I am parent, pid:%d, ppid:%d\n",getpid(),getppid());
		sleep(1);
	}

	return 0;
}
