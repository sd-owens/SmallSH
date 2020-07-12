#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define OPEN_FLAGS  (O_RDWR | O_CREAT | O_TRUNC)
#define FILE_PERMS  (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)  /* rw-rw-rw- */


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
//TODO needs to handle input redirection
void redirectInput(char *argv[])
{
    // int targetFD = open(argv[2], OPEN_FLAGS, FILE_PERMS);

    // if(targetFD == -1)
    // {
    //     perror("open()");
    //     exit(1);
    // }

    // int output = dup2(targetFD, 1);  //redirect stdout to write to targetFD  
    // if (output == -1) {
    //     perror("dup2"); 
    //     exit(2); 
    // }

}

void redirectOutput(char *argv[])
{

    int targetFD = open(argv[2], OPEN_FLAGS, FILE_PERMS);

    if(targetFD == -1)
    {
        perror("open()");
        exit(1);
    }

    int output = dup2(targetFD, 1);  //redirect stdout to write to targetFD  
    if (output == -1) {
        perror("dup2"); 
        exit(2); 
    }

}

int execute( char *argv[] )
{
    int childStatus;

    if(strcmp(argv[0], "cd") == 0 || strcmp(argv[0], "chdir") == 0)
    {
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
                // setup redirection of child before calling exec()
                if(argv[1] != NULL && strcmp(argv[1], ">") == 0)
                {
                    redirectOutput(argv);
                    argv[1] = NULL;  // replace > with NULL for execvp()
                } 
                else if (argv[1] != NULL && strcmp(argv[1], "<") == 0)
                {
                    redirectInput(argv);
                    argv[1] = NULL;  // replace > with NULL for execvp()
                }
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

void handle_SIGINT(int signo)
{
    //char* message = "termindated by process \n";
    char message[40];

    sprintf(message, "terminated by signal %d\n", signo);

    write(STDOUT_FILENO, message, 40);
    raise(SIGUSR2);
    sleep(10);
}

void handle_SIGUSR2(int signo){
	char* message = "Caught SIGUSR2, exiting!\n";
	write(STDOUT_FILENO, message, 26);
	exit(0);
}


int init()
{
    // Setup Custom Single Handlers for Small Shell Program (Parent Process Only)
    struct sigaction SIGINT_action = {0}, 
                     SIGUSR2_action = {0},
                     ignore_action = {0};

    // Fill out the SIGINT_action struct
    // Register handle_SIGINT as the signal handler
	SIGINT_action.sa_handler = handle_SIGINT;
    // Block all catchable signals while handle_SIGINT is running
	sigfillset(&SIGINT_action.sa_mask);
    // No flags set
	SIGINT_action.sa_flags = 0;
    sigaction(SIGINT, &SIGINT_action, NULL);

    // Fill out the SIGUSR2_action struct
    // Register handle_SIGUSR2 as the signal handler
	SIGUSR2_action.sa_handler = handle_SIGUSR2;
    // Block all catchable signals while handle_SIGUSR2 is running
	sigfillset(&SIGUSR2_action.sa_mask);
    // No flags set
	SIGUSR2_action.sa_flags = 0;
    sigaction(SIGUSR2, &SIGUSR2_action, NULL);

    // The ignore_action struct as SIG_IGN as its signal handler
	ignore_action.sa_handler = SIG_IGN;

    // Register the ignore_action as the handler for SIGTERM, SIGHUP, SIGQUIT. So all three of these signals will be ignored.
	sigaction(SIGTERM, &ignore_action, NULL);
	sigaction(SIGHUP, &ignore_action, NULL);
	sigaction(SIGQUIT, &ignore_action, NULL);

    welcome();
    run();

}