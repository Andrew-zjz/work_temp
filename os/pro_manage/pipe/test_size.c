#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
 
int main()
{
    int pfd[2];
    int ret = pipe(pfd);
    if (ret != 0) {
        perror("pipe");
        exit(-1);
    }
 
pid_t pid = fork();
    if (pid == 0) {
        close(pfd[1]);
		sleep(5);
        return 0;
    } else if (pid > 0) {
        close(pfd[0]);
        char buf[] = "a";
        for (int i = 0; ; i++) {
            write(pfd[1], buf, sizeof(buf[0]));
            printf("write pipe %d, total %d\n", i+1, (i+1)*sizeof(buf[0]));
        }
        pid_t cpid = waitpid(-1, NULL, 0);
    } else {
        perror("fork");
    }
    return 0;
}
