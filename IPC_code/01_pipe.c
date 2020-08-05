#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc,char** argv)
{
	int fd[2];
	pipe(fd);

	pid_t pid = fork();

	if(pid == 0){
		sleep(2);
		int ret = write(fd[1],"hello~\n",8);
		if(ret < 0)
			perror("write erro");
	}
	else{
		char buff[10] = {0};
		int ret = read(fd[0],buff,sizeof(buff));

		if(ret < 0)
			perror("read erro");
		else
			printf("%s",buff);

		wait(0);
	}

	return 0;

}
