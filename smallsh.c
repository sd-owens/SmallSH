#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>



int run()
{
    fprintf(stdout, "Welcome to S2: Small Shell\n");
    fprintf(stdout, ":\n");
    fprintf(stderr, "Doing some child stuff...\n");


}


int init()
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

            run();

            exit(2);
            break;
        default:
            spawnPid = waitpid(spawnPid, &childStatus, 0);
            printf("PARENT(%d): child(%d) terminated.  Exiting\n", getpid(), spawnPid);
            exit(EXIT_SUCCESS);
            break;
    }

}

int main ()
{

    int test = 1;

   
    init();

    
}