#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_INPUT_LENGTH 2048
#define MAX_ARGS 512
#define MAX_PID_LENGTH 8

struct InputInfo {
    char* command;
    char** args;
    int args_count;
    char* rdr_in_path;
    char* rdr_out_path;
    bool isBackground;
};

typedef struct InputInfo InputInfo;

InputInfo* buildInputStruct(void);
InputInfo* parseInput(char*, char*);
void printInputInfo(InputInfo*);
void freeInputInfo(InputInfo*);

bool inputExpansion(char*, char*, char**);