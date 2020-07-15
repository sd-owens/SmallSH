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

void print_exit_status(int spawnPid, int child_status)
{
	if(WIFEXITED(child_status))
        {
            int lastExit = WEXITSTATUS(child_status);
            fprintf(stdout, "background pid is %d is done: exit value %d\n", spawnPid, lastExit);
            fflush(stdout);
        }
        else
        {
            int signal = WTERMSIG(child_status);
            fprintf(stdout, "background pid is %d is done: terminated by signal %d\n", spawnPid, signal);
            fflush(stdout);
        }
}

void handle_SIGTSTP(int signo)
{
    char *message;

    if(allowBackground == true)
    {
        allowBackground = false;
        message = "\nEntering foreground-only mode (& is now ignored)\n: ";
        write(STDOUT_FILENO, message, 52);
        fflush(stdout);
    } 
    else 
    {
        allowBackground = true;
        message = "\nExiting foreground-only mode\n: ";
        write(STDOUT_FILENO, message, 32);
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

int execute( char *argv[], bool background, int *signal, int *exit_status, struct sigaction *SIGINT_action)
{

    int child_status, numPids = 0;

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
                exit(1);
                break;

            // spawn id is 0 in the child process.   
            case 0:

                // Reset child signal handler for SIGINT to default values.
	            SIGINT_action->sa_handler = SIG_DFL;
	            //sigfillset((SIGINT_action->sa_mask));
	            //SIGINT_action->sa_flags = 0;
                sigaction(SIGINT, SIGINT_action, NULL);

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
                perror(argv[0]);
                exit(2);
                break;

            default:

                if(background && allowBackground)
                {
                    waitpid(spawnPid, &child_status, WNOHANG);
                    printf("background pid is %d\n", spawnPid);
				    fflush(stdout);
                    numPids++;
                }
                else 
                {
                    waitpid(spawnPid, &child_status, 0);
                    if(strcmp(argv[0], "kill") == 0)
                    {
                        //TODO, bad logic, needs to clean up;
                    }
                    else if(WIFEXITED(child_status))
                    {
                        *exit_status = WEXITSTATUS(child_status);
                    }
                    else
                    {
                        *signal = WTERMSIG(child_status);
                        fprintf(stdout, "terminated by signal %d\n", signal);

                    }
                }
                   
           

            // Check for terminated child background processes.    
            while ((spawnPid = waitpid(-1, &child_status, WNOHANG)) > 0)
            {
                if(numPids != 0){
                    print_exit_status(spawnPid, child_status);
                    fflush(stdout);
                }
                numPids--;
                
            }
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


int run(struct sigaction *SIGINT_action)
{
    size_t nbytes = 2048;
    char* input = malloc(nbytes);
    int signal = 0, exit_status = 0, argc = 0, arg_size = 64;
    char* argv[512];
    bool loop = true, background = false;

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
        if(strcmp(argv[0], "#") == 0 || strcmp(argv[0], "\n") == 0)
        {
            continue;
        }
        // Exit Command
        else if (strcmp(argv[0], "exit") == 0)
        {
            //TODO needs to check for background processes and kill them.
            loop = false;
        }
        // Status Command
        else if(strcmp(argv[0], "status") == 0) 
        {
            if (signal)
            {
                fprintf(stderr, "terminated by signal %d\n", signal);
                signal = 0;  //reset signal to false (0)
                fflush(stderr);
            }
            else
            {
                fprintf(stderr, "exit value %d\n", exit_status);
                fflush(stderr);
            }
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
                // check if command should be run in the background and set flag
                if(strcmp(argv[argc - 1], "&") == 0)
                {
                    background = true;
                    //replace it with a NULL value for passing to exec
                    argv[argc - 1] = NULL;
                }
                else 
                {
                    // set last char* to NULL for passing to execvp
                    argv[argc] = NULL;  
                    
                }    
                //print_array(argc, argv);
                execute(argv, background, &signal, &exit_status, SIGINT_action);
                
                
        }
        //reset argc and background for next iteration
        argc = 0;
        background = false;
        fflush(stdout);
    }
    
    free(input);
    // 0 sends SIGTERM to all processes within parent process group.
    kill(0, SIGTERM);
    exit(EXIT_SUCCESS);
    
}

int init()
{
    // Redirct ctrl-C to ignore signal in parent process (shell)
    struct sigaction SIGINT_action = {0};
	SIGINT_action.sa_handler = SIG_IGN;
	sigfillset(&SIGINT_action.sa_mask);
	SIGINT_action.sa_flags = 0;
    sigaction(SIGINT, &SIGINT_action, NULL);

    // Redict ctrl-Z to handle_SIGSTP
    struct sigaction SIGTSTP_action = {0};
    //memset(&SIGTSTP_action, 0, sizeof(SIGTSTP_action));
    SIGTSTP_action.sa_handler = handle_SIGTSTP;
	sigfillset(&SIGTSTP_action.sa_mask);
	SIGTSTP_action.sa_flags = SA_RESTART;
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

    welcome();
    run(&SIGINT_action);

}