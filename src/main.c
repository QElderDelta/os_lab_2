#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>

#define FILE_NAME_LENGTH 129

void readLine(char* result) {
    char c;
    int byteCounter = 0;
    while(byteCounter < FILE_NAME_LENGTH) {
        if(read(STDIN_FILENO, &c, 1) == -1) {
            perror("Reading failed");
        }
        if(c == '\n') {
            break;
        } else {
            result[byteCounter++] = c;
        }
    }
    result[byteCounter] = '\0';
}

int main() {
    pid_t pid;
    char c;
    int numberOfProcesses = 1;
    char buf[FILE_NAME_LENGTH];
    readLine(buf);
    if(atoi(buf) > 0) {
        numberOfProcesses = atoi(buf);
    } else {
        printf("Invalid argment - must be a positive number\n");
        exit(-1);
    }
    int pipes[numberOfProcesses][2];
    char fileNames[numberOfProcesses][FILE_NAME_LENGTH];
    int processNumber = 0;
    for(int i = 0; i < numberOfProcesses; ++i) {
        readLine(buf);
        strcpy(fileNames[i], buf);
    }
    for(int i = 0; i < numberOfProcesses; ++i) {
        if(pipe(pipes[i]) == -1) {
            perror("Pipe failed");
        }
    }
    for(int i = 0; i < numberOfProcesses; ++i) {
       pid = fork(); 
       if(pid == 0) {
            processNumber = i;
            break;
       } else if (pid == -1) {
            perror("Fork failed");
            return -1;
       }
    }
    if(pid > 0) {
        for(int i = 0; i < numberOfProcesses; ++i) {
           close(pipes[i][0]);
        }
        while(read(STDIN_FILENO, &c, 1) > 0) {
            if(write(pipes[processNumber][1], &c, 1) == -1) {
                perror("Writing failed");
            }
            processNumber++;
            processNumber %= numberOfProcesses;
        }
        for(int i = 0; i < numberOfProcesses; ++i) {
            char terminator = '\0';
            if(write(pipes[i][1], &terminator, 1) == -1) {
                perror("Writing failed");
            }
            close(pipes[i][1]);
        }
    } else if(pid == 0) {
        int fd = open(fileNames[processNumber], O_WRONLY, O_CREAT, S_IRWXU);
        if(ftruncate(fd, 0) == -1) {
            printf("errno %i\n", errno);
            perror("ftruncate failed");
        }
        while(1) {
            char c;
            close(pipes[processNumber][1]);
            if(read(pipes[processNumber][0], &c, 1) == -1) {
                perror("Reading failed");
            }
            if(c == '\0') {
                close(pipes[processNumber][0]);
                close(fd);
                exit(0);
            } else {
                if(write(fd, &c, 1) == -1) {
                    perror("Writing failed");
                }
            }
        }     
    }
    return 0;
}
