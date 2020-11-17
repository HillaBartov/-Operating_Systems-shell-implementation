/******************************************
* Student name: Hilla Bartov
* Student ID: 315636779
******************************************/
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <limits.h>

int main() {
    //home directory
    char *home = getenv("HOME");
    //current directory
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    //previous directory
    char prev[PATH_MAX];
    char allCommands[100][100];
    int pids[100];
    int commandsCounter = 0;
    //execute shell
    while (1) {
        char command[100];
        char *c;
        bool background = false;
        int status;
        printf("> ");
        //get command from user
        fgets(command, 100, stdin);
        if (strcmp(command, "\n") == 0) {
            continue;
        }
        if (strstr(command, "exit") != NULL) {
            printf("%d\n", (int) getpid());
            break;
        }
        //handle cd command
        if (strstr(command, "cd") != NULL) {
            printf("%d\n", (int) getpid());
            //handle too many arguments
            char temp[100];
            strcpy(temp, command);
            int i = 0;
            char *tok = strtok(temp, " ");
            while (tok != NULL) {
                i++;
                tok = strtok(NULL, " ");
            }
            //when user entered more then one argument
            if (i > 2) {
                fprintf(stderr, "Error: Too many arguments\n");
                continue;
            }
            //handle cd flags
            if (strchr(command, '~') != NULL) {
                char *token;
                //get the path if exist
                token = strtok(command, "~");
                token = strtok(NULL, "\n");
                //when not only home directory is asked
                if (token != NULL) {
                    //concatenate path to "home" in order to get the full directory
                    strcat(home, token);
                }
                if (chdir(home) == -1) {
                    fprintf(stderr, "Error: No such file or directory\n");
                } else {
                    //save previous directory
                    strcpy(prev, cwd);
                }
            } else if (strchr(command, '-') != NULL) {
                //change to previous directory
                chdir(prev);
                strcpy(prev, cwd);
                //regular cd command, and "cd .."
            } else {
                char *token;
                token = strtok(command, " ");
                token = strtok(NULL, "\n");
                if (chdir(token) == -1) {
                    fprintf(stderr, "Error: No such file or directory\n");
                } else {
                    strcpy(prev, cwd);
                }
            }
            //save current directory
            getcwd(cwd, sizeof(cwd));
            continue;
        }
        //check for the way of execution- foreground\background
        if (strchr(command, '&') != NULL) {
            background = true;
        }
        //take input without "ground" symbol
        c = strtok(command, "&");

        pid_t pid = fork();
        if (pid == 0) {//child
            if (strstr(command, "jobs") != NULL) {
                int i;
                //go over the commands array and check for the running ones
                for (i = 0; i < commandsCounter; ++i) {
                    if (kill(pids[i], 0) == 0) {
                        printf("%d %s\n", pids[i], allCommands[i]);
                    }
                }
                //kill child
                return 0;
            } else if (strstr(command, "history") != NULL) {
                int i;
                strcpy(allCommands[commandsCounter], "history");
                pids[commandsCounter] = getpid();
                commandsCounter++;
                //go over the commands array and check for the running ones
                for (i = 0; i < commandsCounter; ++i) {
                    if (kill(pids[i], 0) == 0) {
                        printf("%d %s RUNNING\n", pids[i], allCommands[i]);
                    } else {
                        printf("%d %s DONE\n", pids[i], allCommands[i]);
                    }
                }
                //kill child
                return 0;
            } else {
                printf("%d\n", (int) getpid());
                //get the first string for command
                char *getCommand;
                getCommand = strtok(c, " ");
                //when the command has no arguments
                if (strchr(getCommand, '\n') != NULL) {
                    getCommand[strlen(getCommand) - 1] = 0;
                }
                char *argv[100];
                argv[0] = getCommand;
                int i = 1;
                char *token;
                //get other arguments and put in the string array argv
                token = strtok(NULL, " ");
                int quotMarks = 0;
                while (token != NULL) {
                    //handle last argument
                    if (strchr(token, '\n') != NULL) {
                        token[strlen(token) - 1] = 0;
                    }
                    //handle echo command, first quotation mark
                    if (strstr(token, "\"") != NULL && quotMarks == 0) {
                        token++;
                        quotMarks++;
                    }
                    //second quotation mark
                    if (strstr(token, "\"") != NULL) {
                        token[strlen(token) - 1] = 0;
                    }
                    argv[i] = token;
                    i++;
                    token = strtok(NULL, " ");
                }
                argv[i] = NULL;
                if ((execvp(getCommand, argv) == -1)) {
                    fprintf(stderr, "Error in system call\n");
                }
            }
        } else if (pid > 0) {
            //foreground shell
            if (!background) {
                wait(NULL);
            }
            //save each pid and command for jobs and history
            pids[commandsCounter] = pid;
            char *temp = strtok(command, "\n");
            strcpy(allCommands[commandsCounter], temp);
            commandsCounter++;
        } else if (pid < 0) {
            fprintf(stderr, "Error in system call\n");
        }
    }
    return 0;
}
