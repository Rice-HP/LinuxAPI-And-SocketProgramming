#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>

void catch(int signo){
	printf("begin catch signal : %d\n",signo);
	sleep(3);
	printf("end catch signal : %d\n",signo);
}

int main(int argc,char** argv)
{
	struct sigaction act;
	act.sa_flags = 0;
	act.sa_handler = catch;
	//清空
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask,SIGQUIT);
	//sigaddset(&act.sa_mask,SIGINT);

	//sigprocmask(SIG_BLOCK,&act.sa_mask,NULL);
	sigaction(SIGTSTP,&act,NULL);
	//sigaction(SIGTSTP,&act,NULL);

	while(1){
		printf("runing...\n");
		sleep(1);
	}
	return 0;
}
