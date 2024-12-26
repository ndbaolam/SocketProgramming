#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    printf("Hello\n");
    pid_t pid = fork();
    printf("%d\n", pid);

    if (pid == 0) {
        // Child process
        printf("This is the child process.\n");
    } else if (pid > 0) {
        // Parent process
        int status;
        wait(&status); // Wait for child to terminate
        if (WIFEXITED(status)) {
            printf("Child exited with status %d\n", WEXITSTATUS(status));
        }
        printf("This is the parent process. Child PID: %d\n", pid);        
    } else {
        // Fork failed
        perror("Fork failed");
        return 1;
    }    

    return 0;
}
