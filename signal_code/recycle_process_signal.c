#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>

void recycle_child(int signo){
	printf("    begin...\n");

	pid_t pid;

	while( (pid = waitpid(-1,NULL,WNOHANG)) > 0){
		printf("\tpid : %d ok~ \n",pid);
	}

	printf("    end... \n");

}
int main(int argc,char** argv)
{
	sigset_t new_sigset,old_sigset;
	sigaddset(&new_sigset,SIGCHLD);
	//设置阻塞
	sigprocmask(SIG_BLOCK,&new_sigset,&old_sigset);

	int i;
	for(i = 0; i < 5; i++){
		pid_t pid = fork();
		if(pid == 0){
			printf("child_%d pid =%d\n",i,getpid());
			break;
		}
	}

	if(i < 5){
		//sleep(i);
	}
	else{
		//sleep(1);
		struct sigaction act;
		act.sa_flags = 0;
		act.sa_handler =  recycle_child;
		sigemptyset(&act.sa_mask);

		sigaction(SIGCHLD,&act,NULL);

		//恢复
		sigprocmask(SIG_SETMASK,&old_sigset,NULL);

		printf("I am father,pid = %d\n",getpid());

		while(1){
			sleep(1);
		}
	}
	
	return 0;

}
