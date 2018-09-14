#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int parse_cmd(char * cmd, char ** parsed) {
        char *p;
        int i = 0;
        char delim[] = " \n";
        //parsed = (char **)malloc(256 * 10 * sizeof(char));
        //for (i = 0; i < 10; i++) {
        //      parsed[i] = NULL;
        //}
        for (p = strtok(cmd, delim); p != NULL; p = strtok(NULL, delim)) {
                parsed[i++] = p;
        }
        return i;
}

static void run_cmd(char *cmd) {
	const pid_t pid = fork();
	int status;
	if (pid < 0) {
		printf("fork() error\n");
	}
	if (pid) {
		waitpid(pid, &status, 0);
                printf("Return code: %d\n", WEXITSTATUS(status));
		return;
	}
	
	printf("I'm a child!\n");
        char ** args = (char**)malloc(256 * 10 * sizeof(char));
        int size = parse_cmd(cmd, args);
        args = realloc(args, 256 * size);

	execvp(args[0], args);
	free(args);
	exit(0);
	printf("exec* failed\n");
}

int main() {
	while(1) {
		char *cmd;
		cmd = (char *)malloc(256 * sizeof(char));
		fgets(cmd, 256, stdin);
		run_cmd(cmd);
		//printf("%s\n", cmd);
		free(cmd);
	}
	return 0;
}
