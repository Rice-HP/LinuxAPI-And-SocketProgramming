#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
	pid_t pid = fork();

	if(pid == 0){
		printf("child(pid %d) about to die\n",getpid());
		sleep(5);
		//exit(77);
		return 66;
	}
	else{
		printf("parent(pid %d) begin \n",getpid());
		while(1);

		int wstatus;
		pid_t p = wait(&wstatus);
		
		if(WIFEXITED(wstatus))
			printf("your son(%d) die,return status=%d\n",p,WEXITSTATUS(wstatus));
		
		if(WIFSIGNALED(wstatus))
			printf("your son(%d) kill by signal %d\n",p,WTERMSIG(wstatus));
			
	}

	return 0;
}
