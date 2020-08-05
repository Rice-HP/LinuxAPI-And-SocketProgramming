#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc,char **argv)
{
	if(argc != 2){
		printf("agr erro");
		return -1;
	}
	chdir(argv[1]);
	char buff[256];
	printf("return: [%s] \nbuff : [%s]\n",getcwd(buff,sizeof(buff)));

	return 0;
}
