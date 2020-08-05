#include <unistd.h>
#include <stdio.h>

int main(){
	execl("/bin/ls", "abcdefg", "-l", "--color=auto", NULL);
	perror("ls");
	printf("not back\n");
	return 0;
}

