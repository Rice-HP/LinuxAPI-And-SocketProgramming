#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

int main(int argc,char** argv)
{
	if(argc != 2){
		printf("argc erro.\n");
		return -1;
	}
	printf("pid = %d begin write.\n",getpid());
	int fd = open(argv[1],O_WRONLY);
	if(fd < 0)
		perror("open erro");
	printf("pid = %d end write.\n",getpid());
	
	int n = 21;
	char buff[10];
	while(n--){
		sprintf(buff,"n = %02d\n",n);
		write(fd,buff,strlen(buff));
		sleep(1);
	}
	close(fd);
	return 0;

}
