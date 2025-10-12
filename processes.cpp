/*
 * Psuedocode:
 * Creates 3 child processes to process
 * the command: ps -ef | grep <keyword> | wc -l
 *               ggc  (pipe)   gc  (pipe)   c
 *
 * Works in reverse ordder
 *      ggc --> ps, gc --> grep, c --> wx -l, parent --> wait()
 *
 * fds[0] --> first pipe between "ps" and "grep"
 * fds[1] --> second pipe between "grep" and "wc"
 *
 */


#include <sys/types.h>   // for fork, wait
#include <sys/wait.h>    // for wait
#include <unistd.h>      // for fork, pipe, dup, close
#include <stdio.h>       // for NULL, perror
#include <stdlib.h>      // for exit
#include <iostream>      // for cout

using namespace std;

int main( int argc, char** argv ) {
    int fds[2][2];
    int pid;

    if (argc != 2) {
        cerr << "Usage: processes command" << endl;
        exit(-1);
    }

    // Create both pipes BEFORE any fork
    pipe(fds[0]); // ps -> grep
    pipe(fds[1]); // grep -> wc

    if ((pid = fork()) < 0) {
        perror("fork error");
    }
    else if (pid == 0) {
        // CHILD PROCESS (ps)
        if ((pid = fork()) == 0) {
            // GREAT-GRANDCHILD -> ps
            dup2(fds[0][1], STDOUT_FILENO); // ps output to pipe0
            close(fds[0][0]); close(fds[0][1]);
            close(fds[1][0]); close(fds[1][1]);
            execlp("ps", "ps", "ef", (char*)NULL);
            perror("exec ps failed");
            exit(1);
        } else {
            // GRANDCHILD -> grep
            if ((pid = fork()) == 0) {
                dup2(fds[0][0], STDIN_FILENO);  // read from ps
                dup2(fds[1][1], STDOUT_FILENO); // output to wc
                close(fds[0][0]); close(fds[0][1]);
                close(fds[1][0]); close(fds[1][1]);
                execlp("grep", "grep", argv[1], (char*)NULL);
                perror("exec grep failed");
                exit(1);
            } else {
                // GREAT-GRANDCHILD -> wc
                dup2(fds[1][0], STDIN_FILENO);  // read from grep
                close(fds[0][0]); close(fds[0][1]);
                close(fds[1][0]); close(fds[1][1]);
                execlp("wc", "wc", "-l", (char*)NULL);
                perror("exec wc failed");
                exit(1);
            }
        }
    }
    else {
        // PARENT
        wait(NULL);
        cout << "commands completed" << endl;
    }
    /*
    // fds[0] connect child → grandchild (ps->grep)
    //fds[1] connect grandchild → great-grandchild (grep-wc)
    int fds[2][2];
    int pid;



    // fork a child
    if ( ( pid = fork( ) ) < 0 ) {
        perror( "fork error" );
    }
    // child does pipeline work
    else if ( pid == 0 ) {
        // CHILD PROCESS
        // create a pipe using fds[0], pipe for ps --> grep
        pipe(fds[0]);

        if ((pid = fork()) == 0) {
            // GREAT GRAND-CHILD PROCESS
            // create another pipe using fds[1]
            pipe(fds[1]);

            if ((pid = fork()) == 0) {
                // STDIN_FILENO (0) → reads from keyboard
                // STDOUT_FILENO (1) writes to screen
                // 0 = read end of pipe, 1 = write end of pipe
                dup2(fds[0][1], STDOUT_FILENO);
                close(fds[0][0]); // closes the unused read end
                close(fds[0][1]); // closes write
                // -e → show all processes
                //-f → show full format listing
                execlp("ps", "ps", "ef", (char*)NULL);
                perror("the exec ps command failed");
                exit(1);
            } else {
                // GRAND-CHILD PROCESS
                dup2(fds[0][0], STDIN_FILENO);
                dup2(fds[1][1], STDOUT_FILENO);
                close(fds[0][0]);
                close(fds[0][1]);
                close(fds[1][0]); // close read end (pipe1)
                close(fds[1][1]); // close write end (pipe1)
                execlp("grep", "grep", argv[1], (char*)NULL);
                perror("exec grep failed");
                exit(1);
            }
        } else {
            // GRAND CHILD
            // read from pipe1
            dup2(fds[1][0], STDIN_FILENO);
            close(fds[0][0]);
            close(fds[0][1]);
            close(fds[1][0]);
            close(fds[1][1]);
            execlp("wc", "wc", "-l", (char*)NULL);
            perror("the exec wc command failed");
            exit(1);
        }

    }
    else {
        // PARENT
        wait( NULL );
        cout << "commands completed" << endl;
    }
     */
}

