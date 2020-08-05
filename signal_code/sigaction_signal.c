#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>

void catch(int signo){
	printf("catch signal : %d\n",signo);
}

int main(int argc,char** argv)
{
	struct sigaction act;
	act.sa_flags = 0;
	act.sa_handler = catch;
	//清空
	sigemptyset(&act.sa_mask);

	sigaction(SIGALRM,&act,NULL);
	sigaction(SIGTSTP,&act,NULL);

	struct itimerval it = {{2,0},{3,0}};

	setitimer(ITIMER_REAL,&it,NULL);

	while(1){
		printf("runing...\n");
		sleep(1);
	}
	return 0;
}
