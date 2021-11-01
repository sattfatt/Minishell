/*  
    Author: Satyam Patel
    This is an implementation of a shell. It has limited functionality but does support
    -Native execution of cd, and exit
    -Execution of all linux commands as child processes (foreground and background)
    -Input and output redirection
    -Signal handling of SIGINT and SIGTSTP

    syntax:
    command [args ...] [< input_file] [> output_file] [&]

    Max input length = 2048
    Max args = 512
    Max path length = 2048
*/


To compile, just do:
$ make

To clean, just do:
make clean

If that doesnt work then the compilation command is:
gcc -std=gnu99 -g3 -Wall -o smallsh smallsh.c parser.c utils.c signals.c

To run, just do:
./smallsh

There are a bunch of warnings, but ignore them.