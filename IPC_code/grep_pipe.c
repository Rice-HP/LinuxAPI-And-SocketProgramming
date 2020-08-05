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
		sleep(1);
		//重定向输出至写端
		dup2(fd[1],STDOUT_FILENO);
		//执行ps
		execlp("ps","ps","ajx",NULL);
	}
	else{
		//必须关闭写入端,阻塞才取消
		close(fd[1]);
		dup2(fd[0],STDIN_FILENO);
		execlp("grep","grep","bash",NULL);
	}

	return 0;

}
