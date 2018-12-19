#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/select.h>

#define SIZE 256

int main() {
	int fd = open("server", O_RDWR);
	int fl = 1;
	char fifo1[SIZE];
	char fifo2[SIZE];
	int fd1, fd2;
	char buf[SIZE];

	while (fl) {
		fgets(buf, SIZE, stdin);
		buf[strlen(buf) - 1] = '\0';
		char token[SIZE];
		char stor[SIZE];
		strcpy(stor, buf);
		strcpy(token, strtok(stor, " "));
		if (!strcmp(token, "REGISTER")) {
			strcpy(fifo1, strtok(NULL, " "));
			strcpy(fifo2, strtok(NULL, " "));
			mknod(fifo1, S_IFIFO | 0666, 0);
			fd1 = open(fifo1, O_RDWR);
			mknod(fifo2, S_IFIFO | 0666, 0);
			fd2 = open(fifo2, O_RDWR);
			int flags = fcntl(fd2, F_GETFL, 0);
			fcntl(fd2, F_SETFL, flags | O_NONBLOCK);
			
			write(fd, buf, SIZE);
			printf("Want to register\n");
			fl = 0;
		}
	}
	int err = 0;
	while (err <= 0) {
		err = read(fd2, buf, SIZE);
	}
	buf[err] = '\0';
	printf("%s\n", buf);

	while (1) {
		int res;
		fgets(buf, SIZE, stdin);
		buf[strlen(buf) - 1] = '\0';
		write(fd1, buf, SIZE);
		printf("Message sent\n");
		err = 0;
		while (err <= 0) {
			err = read(fd2, buf, SIZE);
		}
		buf[err] = '\0';
		printf("File received: %s\n", buf);
	}
	return 0;
}
