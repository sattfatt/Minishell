#include <stdbool.h>

void clear_stdin(void);
void write_int(int);
void onExit(int);
void ignore_SIGINT(void);
void default_SIGINT(void);
void ignore_SIGTSTP(void);
void catch_SIGTSTP(void);
void toggle_background_mode(int);

