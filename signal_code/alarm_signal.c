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
	signal(SIGALRM,catch);

	struct itimerval it = {{2,0},{3,0}};

	setitimer(ITIMER_REAL,&it,NULL);
	
	int n = 21;
	while(n--){
		printf("%d\n",n);
		sleep(1);
	}
	
	return 0;
}
