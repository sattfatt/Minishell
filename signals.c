#include <fcntl.h>
#include <sys/wait.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "signals.h"
#include <stdbool.h>

extern bool g_ignore_bg;

void toggle_background_mode(int signal) {
    g_ignore_bg = !g_ignore_bg;
    if (g_ignore_bg) {
        write(STDOUT_FILENO, "\nEntering foreground-only mode (& is now ignored)\n", 50);
    } else {
        write(STDOUT_FILENO, "\nExiting foreground-only mode\n", 30);
    }
}

void clear_stdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

void write_int(int number) {
    char result[10];
    int idx = 0;
    int num = number;
    do {
        result[idx++] = num % 10 + '0';
        num /= 10;
    } while(num != 0);
    // num is reversed so we gotta reverse it back we can swap till the half point
    for (int j = 0; j < idx/2; j++) {
        char t = result[j];
        result[j] = result[idx - j - 1];
        result[idx - j - 1] = t;
    }

    write(STDOUT_FILENO, result, idx);

    
}

void onExit(int signal) {
    int wstat = 0;
    //char* wstat_s;
    pid_t pid;
    //char* pid_s;
    
    while ((pid = waitpid(-1, &wstat, WNOHANG)) > 0) {
        if(WIFEXITED(wstat)) {
            write(STDOUT_FILENO, "\nBackground pid ", 16);
            write_int(pid);
            write(STDOUT_FILENO, " is done: ", 10);
            write(STDOUT_FILENO, "exit value ", 11);
            write_int(WEXITSTATUS(wstat));
            write(STDOUT_FILENO, "\n: ", 3);
        }
        else {
            fflush(stdout);
            write(STDOUT_FILENO, "Background pid ", 15);
            write_int(pid);
            write(STDOUT_FILENO, " is done: ", 10);
            write(STDOUT_FILENO, "terminated by signal ", 21);
            write_int(WTERMSIG(wstat));
            write(STDOUT_FILENO, "\n", 1);
            fflush(stdout);
        }
    }
    // also clear out stdin and stdout here
    fflush(stdout);
    //clear_stdin();
}

void ignore_SIGINT() {
    struct sigaction ignore_action = {0};
    ignore_action.sa_handler = SIG_IGN;
    sigaction(SIGINT, &ignore_action, NULL);
}

void default_SIGINT() {
    struct sigaction default_action = {0};
    default_action.sa_handler = SIG_DFL;
    sigaction(SIGINT, &default_action, NULL);
}

void ignore_SIGTSTP() {
    struct sigaction ignore_action =  {0};
    ignore_action.sa_handler = SIG_IGN;
    sigaction(SIGTSTP, &ignore_action, NULL);
}

void catch_SIGTSTP() {
    struct sigaction toggle_action = {0};
    toggle_action.sa_handler = toggle_background_mode;
    sigfillset(&toggle_action.sa_mask);
    toggle_action.sa_flags = 0;
    sigaction(SIGTSTP, &toggle_action, NULL);
}