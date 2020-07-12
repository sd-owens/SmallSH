#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


void print_array(int argc, char *argv[])
{

    for (int i = 0; i < argc; i++)
    {
        fprintf(stdout, "argv[%d] = %s\n", i, argv[i]);
        fflush(stdout);
    }
    
}


int welcome()
{
    system("clear");

    fprintf(stdout, "\nWelcome to:\n");   

    fprintf(stdout, " _____                 _ _   _____ _          _ _\n" 
                    "/  ___|               | | | /  ___| |        | | |\n"
                    "\\ `--. _ __ ___   __ _| | | \\ `--.| |__   ___| | |\n"
                    " `--. \\ '_ ` _ \\ / _` | | |  `--. \\ '_ \\ / _ \\ | |\n"
                    "/\\__/ / | | | | | (_| | | | /\\__/ / | | |  __/ | |\n"
                    "\\____/|_| |_| |_|\\__,_|_|_| \\____/|_| |_|\\___|_|_|\n");
                                                  
    fprintf(stdout, "\nA lighter shell, similar to the Bourne Again SHell!\n\n");                                             

    fflush(stdout);
}

int changeDir(char *argv[])
{

    if(argv[1] == NULL || strcmp(argv[1], "~") == 0)
        argv[1] = getenv("HOME");

    int status = chdir(argv[1]);

    switch(status)
    {
        case -1:
            perror("invalid path");
            break;
        default:
            return 0;
    }

}

int execute( char *argv[] )
{
    int childStatus;

    if(strcmp(argv[0], "cd") == 0 || strcmp(argv[0], "chdir") == 0)
    {
        //TODO add call to internal cd comment here.
        changeDir(argv);
    }
    else
    {
        pid_t spawnPid = fork();

        switch(spawnPid)
        {
            // fork() returns -1 if it fails to spawn a new child
            case -1:
                perror("fork() failed!");
                exit(EXIT_FAILURE);
                break;
            // spawn id is 0 in the child process.   
            case 0:
                // printf("Fork successful, CHILD(%d) running\n", getpid());
                // fflush(stdout);
                execvp(argv[0], argv);
                perror("execvp");
                exit(EXIT_FAILURE);
                break;

            default:

                spawnPid = waitpid(spawnPid, &childStatus, 0);
                //TODO need function to wait for background processes to complete.
                //printf("PARENT(%d): child(%d) terminated.  Exiting\n", getpid(), spawnPid);
                break;
        }
    }

    return(0);
}

int freeMem()
{

}

int parse(char *input, char *argv[])
{
        // look for &
        // look for redirection
        // look for $$ for expansion to process ID


}


int run()
{
    size_t nbytes = 2048;
    char* input = malloc(nbytes);
    int argc = 0, arg_size = 64;
    char* argv[512];

    while(true)
    {
        fprintf(stdout, ": ");
        fflush(stdout);

        getline(&input, &nbytes, stdin);    // might need to change to newfd  

        strtok(input, "\n");  // strip off newline char from end of user input.

        argv[0] = (char *) malloc(sizeof(char) * arg_size);
        assert(*argv != NULL);

        argv[argc] = strtok(input, " ");  // break up user input into tokens seperate by spaces.

        while (argv[argc] != NULL)
        {
            argc++;
            argv[argc] = (char *) malloc(sizeof(char) * arg_size);
            assert(argv[argc] != NULL);
            argv[argc] = strtok(NULL, " ");

        }

        // set last char* to NULL for passing to execvp
        argv[argc] = NULL;  

        //print_array(argc, argv);

        execute(argv);

        //reset argc for next iteration
        argc = 0;
    }
    
    free(input);
    
}


int init()
{
    
    welcome();
    run();

}