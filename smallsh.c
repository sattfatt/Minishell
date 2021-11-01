/*  
    Author: Satyam Patel
    This is an implementation of a shell. It has limited functionality but does support
    -Native execution of cd, and exit (clearly)
    -Execution of all linux commands as child processes (foreground and background)
    -Input and output redirection
    -Signal handling

    syntax:
    command [args ...] [< input_file] [> output_file] [&]

    Max input length = 2048
    Max args = 512
    Max path length = 2048
*/

/*
    Main loop of the shell. 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h> 
#include "parser.h"
#include <fcntl.h>
#include "signals.h"


#define MAX_PATH 256

// globals
bool g_ignore_bg = false;

void shellLoop(void) {
    // make current dir variable. (Dynamically allocated)

    char working_dir[MAX_PATH] = {0};
    getcwd(working_dir, MAX_INPUT_LENGTH);

    // get the process id of the shell
    pid_t pid = getpid();
    // get pid as string
    char pid_s[MAX_PID_LENGTH] = {0};
    snprintf(pid_s, MAX_PID_LENGTH, "%d", pid);

    // status variable
    int status = 0;
    int child_status;
    int mode;
    int wait_pid_result;

    // setup sigint ignore
    ignore_SIGINT();
    catch_SIGTSTP();
    // loop
    while(true) {
        // get user input
        char input[MAX_INPUT_LENGTH] = {0};
        printf(": ");
        fflush(stdout);
        fgets(input, MAX_INPUT_LENGTH, stdin);

        // get rid of the newline
        int len = strlen(input);
        // if no input just go again.
        if(len == 1) {
            continue;
        }
        // if we have a # also go again
        if(input[0] == '#') {
            continue;
        }

        input[len-1] = 0;

        // parse that input
        InputInfo *input_info = parseInput(input, pid_s);

        //printInputInfo(input_info);
        //fflush(stdout);
        // check if we need to ignore isBackground flag in input_info
        if (g_ignore_bg) {
            input_info->isBackground = false;
        }

        // Handle built in commands

        if(strcmp(input_info->command, "cd") == 0) {
            if (input_info->args_count == 2) { chdir(input_info->args[1]); getcwd(working_dir, MAX_INPUT_LENGTH); }
            else if (input_info->args_count == 1) { chdir(getenv("HOME")); getcwd(working_dir, MAX_INPUT_LENGTH); }
            else printf("Invalid arguments for cd.\n");
            fflush(stdout);

        }
        // kill all processes in the group to exit. 
        else if (strcmp(input_info->command, "exit")==0){
            kill(0,9);
        } 
        // print out the status based on if child normally or abnormally exited.
        else if (strcmp(input_info->command, "status")==0) {
            if (WIFEXITED(status)) {
                printf("exit value: %d\n", WEXITSTATUS(status));
            }
            else {
                printf("terminated by signal: %d\n",WTERMSIG(status));
            }
            fflush(stdout);
        }

        // otherwise fork and exec the provided command.
        else if (!strcmp(input_info->command, "")==0){

            pid_t spawn_pid = fork();
            
            // for child notifying parent of completion.
            if(input_info->isBackground) signal(SIGCHLD, onExit);

            // used for redirection
            int destFD = NULL;
            int sourceFD = NULL;

            switch(spawn_pid) {
                case -1:
                    perror("fork failed!");
                    break;
                case 0:
                    // setup child signal handler for SIGINT
                    if (input_info->isBackground) {
                        ignore_SIGINT();
                        ignore_SIGTSTP();
                    } else {
                        default_SIGINT();
                        ignore_SIGTSTP();
                    }

                    // do any input/output redirection here with dup2()

                    if (input_info->isBackground) {
                        // if we have no input redirection, get the input from /dev/null
                        if (!input_info->rdr_in_path) {
                            sourceFD = open("/dev/null", O_RDONLY);
                        } else {
                            sourceFD = open(input_info->rdr_in_path, O_RDONLY);
                        }
                    } else {
                        // if not background see if there is input rdr and use that.
                        if (input_info->rdr_in_path) {
                            sourceFD = open(input_info->rdr_in_path, O_RDONLY);
                        }
                    }

                    if (sourceFD == -1) {
                        perror("source open()");
                        exit(1);
                    }

                    if(sourceFD) {
                        int result = dup2(sourceFD, 0);
                        if (result == -1) {
                            perror("source dup2()");
                            exit(2);
                        }
                    }


                    // do the same for output redirection
                    if (input_info->isBackground) {
                        // if we have no input redirection, get the input from
                        if (!input_info->rdr_out_path) {
                            destFD = open("/dev/null", O_WRONLY);
                        } else {
                            destFD = open(input_info->rdr_out_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        }
                    } else {
                        if (input_info->rdr_out_path) {
                            destFD = open(input_info->rdr_out_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        }
                    }
                    if (destFD == -1) {
                        perror("destination open()");
                        exit(1);
                    }

                    if(destFD) {
                        int result = dup2(destFD, 1);
                        if (result == -1) {
                            perror("destination dup2()");
                            exit(2);
                        }
                    }

                    // call exec
                    execvp(input_info->command, input_info->args);
                    // if error then print out error
;
                    perror(input_info->command);

                    exit(1);
                    break;
                default:
                    // check if background process and wait if not.

                    mode = input_info->isBackground ? WNOHANG : 0;

                    wait_pid_result = waitpid(spawn_pid, &child_status, mode);
                     
                    // otherwise just go on with your life. also print the childs pid
                    
                    if (!input_info->isBackground) {
                        // update the status variable with whatever the child reported. (foreground only)
                        status = child_status;
                        if(!WIFEXITED(child_status)) {
                            printf("terminated by signal %d\n", WTERMSIG(child_status));
                            fflush(stdout);
                        }
                    }
                    else {
                        // print the pid of the background process that was just launched.
                        printf("background pid is %d\n", spawn_pid);
                        fflush(stdout);
                    }
            }
        }

        // cleanup
        freeInputInfo(input_info);
        fflush(stdout);
    }
    return;
}

int main(void) {
    shellLoop();
    return 0;
}