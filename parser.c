#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "utils.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>


/* 
    initializes Input info struct members to NULL
*/
InputInfo* buildInputInfoStruct(void) {
    InputInfo* instance = malloc(sizeof(InputInfo));
    instance->command=NULL;
    instance->args=NULL;
    instance->args_count=0;
    instance->rdr_in_path=NULL;
    instance->rdr_out_path=NULL;
    instance->isBackground=false;
    return instance;
}

/*
    Parses the input from the user to get the command and any other inputs like redirects and background flag
    Builds InputInfo struct and points inputInfo to it. So just pass in a pointer (uninitialized).
*/
InputInfo* parseInput(char* input, char* pid_s) {

    // copy the input for tokenization
    char* input_cpy = copyString(input);

    // look for any instance of $$ and with $$ expanded to the pid of the process.
    char* expanded = NULL;
    bool found = false;
    do {
        found = inputExpansion(input_cpy, pid_s, &expanded);
        if (found) {
            // free the unexpanded string and point input_cpy to the expanded one.
            // this may also be a previously expanded string that still has more $$.
            free(input_cpy);
            input_cpy = expanded;
        }
    }
    while(found);
    
    // build a InputInfo struct
    InputInfo* input_info = buildInputInfoStruct();

    // get the first token
    char* token; 
    char* saveptr;
    // this should be the command 
    token = strtok_r(input_cpy, " ", &saveptr);
    if(!token) {
        input_info->command = calloc(2, sizeof(char));
        strcpy(input_info->command, "");
    }
    else {
        input_info->command = calloc(strlen(token + 1), sizeof(char));
        strcpy(input_info->command, token);
    }

    // now get all the args
    char* args[MAX_ARGS] = {0};
    args[0] = input_info->command;
    int count = 1;
    while(token && count < MAX_ARGS) {
        // get the next token seperated by 1 space
        token = strtok_r(NULL, " ", &saveptr);
        if(!token) {
            break;
        }
        if (strcmp(token, ">")==0) {
            // this next token should be the file to write to
            token = strtok_r(NULL, " ", &saveptr);
            input_info->rdr_out_path = calloc(strlen(token+1), sizeof(char));
            strcpy(input_info->rdr_out_path, token);
            continue;
        } else if (strcmp(token, "<")==0) {
            // this next token should be the file to read from
            token = strtok_r(NULL, " ", &saveptr);
            input_info->rdr_in_path = calloc(strlen(token+1), sizeof(char));
            strcpy(input_info->rdr_in_path, token);
            continue;
        }
        // otherwise copy the tokens to args array they are probably arguments to the command.
        args[count] = calloc(strlen(token)+1, sizeof(char));
        strcpy(args[count], token);
        count++;
    }

    // if the last argument is & decrease count by 1 and set isBackgrount to true
    if(strcmp(args[count-1], "&") == 0) {
        input_info->isBackground = true;
        count--;
    }

    // copy the args to the args struct and set the count.
    input_info->args = calloc(count+1, sizeof(char*));
    for (int i=0; i<count; i++) {
        input_info->args[i] = args[i];
    }
    // set the last element to NULL for execv
    input_info->args[count] = NULL;
    input_info->args_count = count;

    // free the input copy
    free(input_cpy);

    return input_info;
}

/*
    prints out the InputInfo struct
*/

void printInputInfo(InputInfo* input) {
    if(input->command) printf("command: %s\n", input->command);
    printf("args: ");
    for (int i; i<input->args_count; i++) {
        printf("%s ", input->args[i]);
    }
    printf("\n");
    printf("num args: %d\n", input->args_count);

    input->rdr_in_path ? 
        printf("rdr in: %s\n", input->rdr_in_path) :
        printf("rdr in: None\n");

    input->rdr_out_path ? 
        printf("rdr out: %s\n", input->rdr_out_path) :
        printf("rdr out: None\n");

    printf("is background: %s\n", input->isBackground?"true":"false");
}

/* 
    Completely frees Input Info struct and checks for NULL entries
*/
void freeInputInfo(InputInfo *input_info) {
    if(input_info->command) free(input_info->command);
 
    // figure out how many args there are
    if(input_info->args) {     
        for(int i = 1; i < input_info->args_count; i++) {
            if(input_info->args[i]) free(input_info->args[i]);
        }
        free(input_info->args);
    }

    if(input_info->rdr_in_path) free(input_info->rdr_in_path);
    if(input_info->rdr_out_path) free(input_info->rdr_out_path);

    free(input_info);
}


/*
    takes in string and expands the first instance of $$ into the string. Note that result will always be allocated.
    So if you are calling this function in a loop make sure to free the result between subsequent calls.
    Returns true if an expansion is performed. Otherwise returns false.
*/
bool inputExpansion(char* original, char* replace, char** result) {
    int found_idx = 0;
    bool found = false;

    // loop through the original and look for $$
    int len = strlen(original);
    // no way we could have $$
    if (len < 2) {
        return false;
    }
    
    // loop through original and look for the index where it occurs
    for (int i=0; i < len-1; i++) {
        if (original[i]=='$' && original[i+1]=='$') {
            found = true;
            found_idx = i;
        }
    }

    // if nothing was found return the false
    if (!found) {
        return false;
    }

    int replace_len = strlen(replace);
    int original_len = strlen(original);

    // otherwise create a new string with the expanded pid.
    int total_size = original_len+replace_len-1;
    char* expanded = calloc(total_size, sizeof(char));


    // loop through original and copy it up untill found_idx
    for (int i=0; i<found_idx; i++) {
        expanded[i] = original[i];
    } 

    // cat the pid
    strcat(expanded, replace);
    
    // copy the rest of the string
    int j = found_idx + replace_len;
    if (found_idx != original_len-2) {
        for(int i=found_idx+2; i < original_len; i++) {
            expanded[j] = original[i];
            j++;
        } 
    }
    
    // set the result and return true
    *result = expanded;
    return true;
}