#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>

int main(int argc,char **argv)
{
	int fd = open("/dev/tty",O_RDWR|O_NONBLOCK);
	char buf[256];
	int ret = 0;
	while(1){
		ret = read(fd,buf,sizeof(buf));
		if(ret < 0){
			perror("read erro");
		}
		else if(buf[0]=='q'){
			break;
		}
		else
		{
			printf("buf:%s\n",buf);
		}
		
		printf("continue...\n");
		sleep(1);
	}
	close(fd);
	return 0;
}

