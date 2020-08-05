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
	printf("pid = %d begin read.\n",getpid());
	int fd = open(argv[1],O_RDONLY);
	if(fd < 0)
		perror("open erro");
	printf("pid = %d end read.\n",getpid());
	
	char buff[20];
	while(1){
		memset(buff,0x00,sizeof(buff));
		int ret = read(fd,buff,sizeof(buff));
		if(ret > 0){
			printf("%s\n",buff);
		}
		else{
			break;
		}
		sleep(1);
	}
	close(fd);
	return 0;

}
