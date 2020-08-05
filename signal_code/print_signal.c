#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

int main(int argc,char** argv)
{
	sigset_t sig_set;
	//清空
	sigemptyset(&sig_set);
	//添加信号
	sigaddset(&sig_set,SIGTSTP);
	sigaddset(&sig_set,SIGQUIT);

	//设置阻塞
	sigprocmask(SIG_BLOCK,&sig_set,NULL);

	while(1){
		sigset_t sig;
		//获取信号
		sigpending(&sig);

		for(int i = 0; i < 31; i++){
			if(sigismember(&sig,i) == 1){
				printf("1");
			}
			else{
				printf("0");
			}

		}
		printf("\n");
		sleep(1);
	}
	
	return 0;

}
