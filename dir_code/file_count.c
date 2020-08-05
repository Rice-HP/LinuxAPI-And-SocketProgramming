#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

int count = 0;

int count_file(const char *path)
{
	printf("\n%s\n\n",path);	

	DIR *dirp = opendir(path);
	if(dirp == NULL){
		perror("open erro");
		return -1;
	}

	struct dirent* cur_file = NULL;
	
	while((cur_file = readdir(dirp)) != NULL){
		if(cur_file->d_type == DT_DIR){ //目录为4
			if(strcmp(cur_file->d_name,".") == 0 || strcmp(cur_file->d_name,"..") == 0)
				continue;

			char newpath[256];
			sprintf(newpath,"%s/%s",path,cur_file->d_name);
			count_file(newpath);
		}
		else if(cur_file->d_type == DT_REG){ //文件为8
			count++;
			printf("%03d : %s\n",count,cur_file->d_name);
		}
	}
	closedir(dirp);

	return 0;	
}

int main(int argc,char **argv)
{
	if(argc != 2){
		printf("agr erro\n");
		return -1;
	}
	
	count_file(argv[1]);		
	printf("total:%d\n",count);

	return 0;
}
