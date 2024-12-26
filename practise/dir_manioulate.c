#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    char buf[1024]; // Buffer to store the current working directory

    if (getcwd(buf, sizeof(buf)) != NULL) {
        printf("Current working directory: %s\n", buf);
    } else {
        perror("getcwd error");
    }

    while(1) {
        char new_dir[20];
        scanf("%s", new_dir);

        if(chdir(new_dir) == 0 && getcwd(buf, sizeof(buf)) != NULL) {
            printf("Current working directory: %s\n", buf);
            system("ls -l");
        } else {
            perror("getcwd error");
        }
    }    

    return 0;
}
