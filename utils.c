#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

/*
    Retruns pointer to a copy of input string in heap.
*/
char* copyString(char* input) {
    char* cp;
    cp = calloc(strlen(input)+1, sizeof(char));
    strcpy(cp, input);
    return cp;
}