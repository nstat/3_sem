#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/wait.h>

#define MEM_SIZE 4096

extern int errno;

int main(int argc, char * argv[]) {
	int fd = open(argv[1], O_CREAT | O_RDONLY);
	if (fd < 0) { 
		printf("open() error\n");
		return -1;
	}
	clock_t	t1 = clock();
	char * mem = (char*)mmap(0, MEM_SIZE, PROT_READ, MAP_SHARED, fd, 0);
	clock_t t2 = clock();
	printf("Mapped at %p\n", mem);
	/*for (int i = 0; i < MEM_SIZE; i++) {
		printf("%c\n", mem[i]);}*/
	close(fd);
	FILE * fdt = fopen("time.txt", "a");
	fprintf(fdt, "mmap\ts: %d\t%lf\n", MEM_SIZE, (t2 - t1) / (double)CLOCKS_PER_SEC);
	fclose(fdt);  	
	pid_t pid = fork();
	int status;
  	if (pid < 0) {
		printf("fork() error\n");
		return -1;
	}
	else if (pid == 0) {
    		sleep(1);
		int fdout = open("received.txt", O_CREAT | O_RDWR, S_IREAD | S_IWRITE);
		t1 = clock();
		if ((write(fdout, mem, MEM_SIZE)) < 0) {
			perror("write()");
			printf("%s\n", strerror(errno));
			exit(-1);
		}
		t2 = clock();
		close(fdout);
		FILE * fdt = fopen("time.txt", "a");
		fprintf(fdt, "mmap\tr: %d\t%lf\n", MEM_SIZE, (t2 - t1) / (double)CLOCKS_PER_SEC);
		fclose(fdt);
		exit(0);
	}
  	else {
		//sleep(1);
		waitpid(pid, &status, 0);
	}
	munmap(mem, MEM_SIZE);
	return 0;
}
