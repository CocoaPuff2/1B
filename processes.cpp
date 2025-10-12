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
    if(argc != 2) { // pass exactly one argument
        cerr << "Usage: processes <command>" << endl;
        exit(-1);
    }

    // fds[0] = pipe ps->grep
    // fds[1] = pipe grep->wc
    int fds[2][2];

    // create the pipes before forking
    if(pipe(fds[0]) == -1) { // ps --> grep
        perror("pipe0 failed"); exit(1);
    }

    if(pipe(fds[1]) == -1) { // grep --> wc
        perror("pipe1 failed"); exit(1);
    }

    pid_t pid;

    //  CHILD PROCESS (wc -l)
    pid = fork();
    if(pid < 0) {
        perror("fork wc failed");
        exit(1);
    }

    else if(pid == 0) { // readend of pipe1 (grep -> wc)
        dup2(fds[1][0], STDIN_FILENO);
        // close not needed pipe ends
        close(fds[0][0]); close(fds[0][1]);
        close(fds[1][0]); close(fds[1][1]);
        execlp("wc", "wc", "-l", (char*)NULL);
        perror("exec wc failed");
        exit(1);
    }

    // GRANDCHILD PROCESS (grep argv[1])
    pid = fork();
    if(pid < 0) {
        perror("fork grep failed");
        exit(1);
    }
    else if(pid == 0) {
        // read from pipe0 (ps --> grep_
        dup2(fds[0][0], STDIN_FILENO);
        // write to pipe1 (grep --> wc)
        dup2(fds[1][1], STDOUT_FILENO);
        close(fds[0][0]); close(fds[0][1]);
        close(fds[1][0]); close(fds[1][1]);
        execlp("grep", "grep", argv[1], (char*)NULL);
        perror("exec grep failed");
        exit(1);
    }

    // GREAT-GRANDCHILD PROCESS (ps -A)
    pid = fork();
    if(pid < 0) {
        perror("fork ps failed");
        exit(1);
    }
    else if(pid == 0) {
        // redirect to pipe0
        dup2(fds[0][1], STDOUT_FILENO);
        close(fds[0][0]); close(fds[0][1]);
        close(fds[1][0]); close(fds[1][1]);
        execlp("ps", "ps", "-A", (char*)NULL);
        perror("exec ps failed");
        exit(1);
    }

    // PARENT: closes all pipe ends immediately
    close(fds[0][0]);
    close(fds[0][1]);
    close(fds[1][0]);
    close(fds[1][1]);

    // wait for children to finish
    for(int i = 0; i < 3; i++) wait(NULL);

    cout << "commands completed" << endl;
    return 0;
}

