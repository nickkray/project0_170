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

struct command{
  char* command;
  char* args[MAX_TOKEN_COUNT];
  int argC;
    
char* file;
    int fileOp; // 1 if < and 2 if >
};


int numCommands = 0;

void  INThandler(int sig){
     signal(sig, SIGTSTP);
     if(pressedOnce)
         exit(0);
     
     pressedOnce = 1;
    
    signal(SIGTSTP, INThandler);
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
    
    struct command commands[100];

  signal(SIGTSTP, INThandler);
  
  char line[MAX_LINE_LENGTH];
  //printf("shell: ");
  while(fgets(line, MAX_LINE_LENGTH, stdin)) {
    pressedOnce = 0;
    // Build the command and arguments, using execv conventions.
    line[strlen(line)-1] = '\0'; // get rid of the new line
    char* command = NULL;
    char* arguments[MAX_TOKEN_COUNT];

    char* meta;
    
    int argument_count = 0;
    
    char* token = strtok(line, " ");
    
    while(token) {
      
      if(strcmp(token, "|") == 0){
	//here we'll save this shit
	commands[numCommands].command = command;
	for(int i=0;i<argument_count;i++)
	  commands[numCommands].args[i] = arguments[i];
	commands[numCommands].argC = argument_count;
	numCommands++;
        commands[numCommands].fileOp = 0; // set default value for next one
	
	command = NULL;
	argument_count = 0;
	token = strtok(NULL, " ");
	continue;
      }
        
        if(strcmp(token, "<") == 0){
            commands[numCommands].fileOp = 1;
            token = strtok(NULL, " ");
            continue;
        }
        
        if(strcmp(token, ">") == 0){
            commands[numCommands].fileOp = 2;
            token = strtok(NULL, " ");
            continue;
        }
        
        if(commands[numCommands].fileOp != 0){  //this was already set above, set filename
            commands[numCommands].file = token;
            token = strtok(NULL, " ");
            continue;
        }
      
      
        if(!command)
            command = token;
        
          arguments[argument_count] = token;
          argument_count++;
        
      token = strtok(NULL, " ");
    }
      
      
      commands[numCommands].command = command;
      for(int i=0;i<argument_count;i++)
          commands[numCommands].args[i] = arguments[i];
      commands[numCommands].argC = argument_count;
      numCommands++;
      
      
      
      
    arguments[argument_count] = NULL;
    if(argument_count>0){
      if (strcmp(arguments[0], "exit") == 0)
	exit(0);
      //runcommand(command, arguments);
    }
    //printf("shell: ");

    for(int i=0;i<numCommands;i++){
    //printf(commands[i].command);

      runcommand(commands[i].command, &commands[i].args);
    }

    
  }
  return 0;
}
