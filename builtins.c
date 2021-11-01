#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

/*
    cats base path with relative one.
*/
char* _cat_dir_paths(char* base, char* new) {

        // get the lengths
        int cur_len = strlen(base);
        int new_len = strlen(new);
        // keep track of whether we need to add /
        bool add_slash_base = false;
        int extra = 1;
        if(base[cur_len-1] != '/') {
            extra++;
            add_slash_base = true;
        }
        int total_len = new_len + cur_len + extra;

        // concat new path to the old one with / if needed.
        char *temp = calloc(total_len, sizeof(char));
        strcpy(temp, base);
        if (add_slash_base) strcat(temp, "/");
        strcat(temp, new);
        free(base);
        return temp;
}


/* 
    checks to see if the path or relative path exists then sets the current path to that path.
*/
void cd(char* newPath, char* *currentPath) {
    //printf("%s\n", newPath);
    // first get rid of the newline if it exists
    int last_idx = strlen(newPath)-1;
    if(newPath[last_idx]=='\n') {
        newPath[last_idx] = 0;
    }

    // if newPath is the empty string then set the current path to the home environment variable.
    if(newPath[0]==0) {
        // get the home env
        char* home = getenv("HOME");
        free(*currentPath);
        *currentPath = calloc(strlen(home)+1, sizeof(char));
        strcpy(*currentPath, home);
        return;
    }

    // check if newPath is just "/" and set that and return.
    if(strcmp(newPath, "/") == 0) {
        free(*currentPath);
        *currentPath = calloc(2, sizeof(char));
        *currentPath[0] = '/';
        return;
    }

    // make a copy of the current path and point finalPath to it.
    char* finalPath = calloc(strlen(*currentPath)+1, sizeof(char));
    strcpy(finalPath, *currentPath);

    // now check if path has .. in the beginning.
    if(strncmp(newPath, "../xx", 3) == 0) {
        // find the last slash /
        int last_slash_idx = 0;
        for (int i=0;i<strlen(finalPath)-1;i++) {
            if (finalPath[i] == '/') {
                last_slash_idx = i;
            }
        }
        // get rid of everything in the current path from last / onwards
        for (int i=last_slash_idx; i<strlen(finalPath); i++) {
            finalPath[i] = 0;
        }

        finalPath = _cat_dir_paths(finalPath, newPath+3);
    } else if (strncmp(newPath, "..", 2) == 0) {
        // if we just have .. then just remove everything after the last slash
        // find the last slash /
        int last_slash_idx = 0;
        for (int i=0;i<strlen(finalPath)-1;i++) {
            if (finalPath[i] == '/') {
                last_slash_idx = i;
            }
        }
        if (last_slash_idx == 0) last_slash_idx++;
        // get rid of everything in the current path from last / onwards
        for (int i=last_slash_idx; i<strlen(finalPath); i++) {
            finalPath[i] = 0;
        }
        // lets get rid of excess memory
        char* temp_path = calloc(strlen(finalPath)+1, sizeof(char));

        strcpy(temp_path, finalPath);

        free(finalPath);
        finalPath = temp_path;
    } else if (newPath[0] == '/') {
        // if we have a exact path set the new path to it.
        free(finalPath);
        finalPath = calloc(strlen(newPath)+1, sizeof(char));
        strcpy(finalPath, newPath);
    } else {
        // otherwise we just cat new path to current path.
        finalPath = _cat_dir_paths(finalPath, newPath);
        printf("%s\n", finalPath);
    }

    // now we validate the finalPath

    DIR* d = opendir(finalPath);
    if(!d) {
        if(finalPath) free(finalPath);
        closedir(d);
        return;
    }
    closedir(d);
    free(*currentPath);
    *currentPath = finalPath;
    return;
}

/*
    Exits the shell and terminates all child processes
*/
void exitAll(pid_t child_pid) {
    kill(0, 9);
}

