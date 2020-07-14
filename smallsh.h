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

bool allowBackground = true;

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

int execute( char *argv[], struct sigaction sa )
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

                sa.sa_handler = SIG_DFL;
                sigaction(SIGINT, &sa, NULL);

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


int run(struct sigaction sa_sigint)
{
    size_t nbytes = 2048;
    char* input = malloc(nbytes);
    int argc = 0, arg_size = 64;
    char* argv[512];
    bool loop = true;

    while(loop)
    {
        fprintf(stdout, ": ");
        fflush(stdout);

        getline(&input, &nbytes, stdin);    // might need to change to newfd  

        strtok(input, "\n");  // strip off newline char from end of user input.

        argv[0] = (char *) malloc(sizeof(char) * arg_size);
        assert(*argv != NULL);

        argv[argc] = strtok(input, " ");  // break up user input into tokens seperate by spaces.

        // skip lines with comments or blank lines.
        if(argv[0] == "#" || argv[0] == "\0")
        {
            continue;
        }
        else if (strcmp(argv[0], "exit") == 0)
        {
            loop = false;
        }
        else 
        {
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
                execute(argv, sa_sigint);
        }
        //reset argc for next iteration
        argc = 0;
    }
    
    free(input);
    
}

void handle_SIGTSTP(int signo)
{
    if(allowBackground == true)
    {
        char message[36];
        sprintf(message, "Foreground-Only Mode (& is ignored)\n");
        write(STDOUT_FILENO, message, 36);
        fflush(stdout);
        allowBackground == false;
    } 
    else 
    {
        char message[30];
        sprintf(message, "Foreground-Only Mode Disabled\n");
        write(STDOUT_FILENO, message, 30);
        fflush(stdout);
        allowBackground == true;
    }
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
    struct sigaction ignore_action = {0};

    // Redirct ctrl-C to handle_SIGINT
    struct sigaction SIGINT_action = {0};
	SIGINT_action.sa_handler = handle_SIGINT;
	sigfillset(&SIGINT_action.sa_mask);
	SIGINT_action.sa_flags = 0;
    sigaction(SIGINT, &SIGINT_action, NULL);

    // Programmer specified handler
    struct sigaction SIGUSR2_action = {0};    
	SIGUSR2_action.sa_handler = handle_SIGUSR2;
	sigfillset(&SIGUSR2_action.sa_mask);
	SIGUSR2_action.sa_flags = 0;
    sigaction(SIGUSR2, &SIGUSR2_action, NULL);

    // The ignore_action struct as SIG_IGN as its signal handler
	ignore_action.sa_handler = SIG_IGN;
	sigaction(SIGHUP, &ignore_action, NULL);
	sigaction(SIGQUIT, &ignore_action, NULL);

    // Redict ctrl-Z to handle_SIGSTP
    struct sigaction sa_sigtstp = {0};
    sa_sigtstp.sa_handler = handle_SIGTSTP;
	sigfillset(&sa_sigtstp.sa_mask);
	sa_sigtstp.sa_flags = 0;
	sigaction(SIGTSTP, &sa_sigtstp, NULL);

    welcome();
    run(SIGINT_action);

}