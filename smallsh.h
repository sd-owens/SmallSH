#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
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
    fprintf(stdout, "Welcome to S2: Small Shell\n");
    fprintf(stdout, ": ");
    fflush(stdout);
   

}

int execute( char *argv[] )
{
    int childStatus;
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
            printf("Fork successful, CHILD(%d) running\n", getpid());
            fflush(stdout);

            execvp(argv[0], argv);
            perror("execvp");
            exit(EXIT_FAILURE);
            break;
        default:

            // if(strcmp(argv[argc - 1], "&") != 0)
            //     spawnPid = waitpid(spawnPid, &childStatus, 0);
            spawnPid = waitpid(spawnPid, &childStatus, 0);

            //TODO need function to wait for background processes to complete.
            printf("PARENT(%d): child(%d) terminated.  Exiting\n", getpid(), spawnPid);
            exit(EXIT_SUCCESS);
            break;
    }


}

int freeMem()
{

}

int parse(char *input, char *argv[])
{



}


int run()
{
    size_t nbytes = 2048;
    char* input = malloc(nbytes);
    int argc = 0, arg_size = 64;
    char* argv[512];

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

    free(input);
    


}


int init()
{
    
    welcome();
    run();

}