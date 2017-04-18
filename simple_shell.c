#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include  <signal.h>

#define MAX_TOKEN_LENGTH 50
#define MAX_TOKEN_COUNT 100
#define MAX_LINE_LENGTH 512

// Simple implementation for Shell command
// Assume all arguments are seperated by space
// Erros are ignored when executing fgets(), fork(), and waitpid().

/**
 * Sample session
 *  shell: echo hello
 *   hello
 *   shell: ls
 *   Makefile  simple  simple_shell.c
 *   shell: exit
 **/

int pressedOnce = 0;

void  INThandler(int sig)
{
     char  c;

     signal(sig, SIGTSTP);
     if(pressedOnce)
       exit(0);
     
     pressedOnce = 1;
     
     //c = getchar();
     //if (c == 'y' || c == 'Y')
     //     exit(0);
     //else
     //     signal(SIGINT, INThandler);
     //getchar(); // Get new line character
}


void runcommand(char* command, char** args) {
    pid_t pid = fork();
    if(pid) { // parent
        waitpid(pid, NULL, 0);
    } else { // child
        execvp(command, args);
    }
}

int main(){

  signal(SIGTSTP, INThandler);
  
  char line[MAX_LINE_LENGTH];
  //printf("shell: ");
  while(fgets(line, MAX_LINE_LENGTH, stdin)) {
    // Build the command and arguments, using execv conventions.
    line[strlen(line)-1] = '\0'; // get rid of the new line
    char* command = NULL;
    char* arguments[MAX_TOKEN_COUNT];
        int argument_count = 0;

        char* token = strtok(line, " ");
	
        while(token) {
            if(!command) command = token;
            arguments[argument_count] = token;
            argument_count++;
            token = strtok(NULL, " ");
        }
        arguments[argument_count] = NULL;
        if(argument_count>0){
            if (strcmp(arguments[0], "exit") == 0)
                exit(0);
            runcommand(command, arguments);
        }
        //printf("shell: "); 
    }
    return 0;
}
