#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include  <signal.h>
#include<semaphore.h>
#include<fcntl.h>
#include <errno.h>

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
    
    char* readFile;
    char* writeFile;
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


/*

 if(commands[i].fileOp == 2){
 int fd = open(commands[i].file, O_CREAT|O_TRUNC|O_WRONLY, 0644);
 dup2(fd, fileno(stdout));
 runcommand(commands[i].command, &commands[i].args);
 close(fd);

*/

void runcommand(char* command, char** args, int fileOp, char* readFile, char* writeFile, int i, int commandNum, int *fd, int *in) {
    
    pid_t pid = fork();
    
    if (pid == -1)
        printf("Unable to create new process: %s\n", strerror(errno));
    
    if(pid) { // parent
        
        close(fd[1]);
        *in = fd[0];
        
        pid_t result = waitpid(pid, NULL, 0);
        
        if(result == -1)
            printf("Child process failed: %s\n", strerror(errno));
        
    } else { // child
        if(fileOp == 2 || fileOp == 3){     //we are writing > or both <>
            int redirectFd = open(writeFile, O_CREAT|O_TRUNC|O_WRONLY, 0644);
            dup2(redirectFd,fileno(stdout));
        }
        if(fileOp == 1 || fileOp == 3){           //we are reading < or both <>
            int redirectFd = open(readFile, O_RDONLY);
            dup2(redirectFd, fileno(stdin));
        }
        
        if(fileOp == 2 || fileOp == 0){
            dup2 (*in, fileno(stdin));
        }
        
        
        if(i < commandNum-1) //commands 0...n-1
            dup2 (fd[1], fileno(stdout));
        
        if(i == commandNum-1) //command n
            dup2(1, fileno(stdout));

        execvp(command, args);
        
        perror(command);
        
    }
}

static const struct command emptyStruct;

int main(){
    
    struct command commands[100];

  signal(SIGTSTP, INThandler);
  
  char line[MAX_LINE_LENGTH];
  //printf("shell: ");
  while(fgets(line, MAX_LINE_LENGTH, stdin)) {
    
    for(int i=0;i<100;i++)
        commands[i] = emptyStruct;
    
    int argument_count = 0;
    
    pressedOnce = 0;
    // Build the command and arguments, using execv conventions.
    line[strlen(line)-1] = '\0'; // get rid of the new line
    char* command = NULL;
    char* arguments[MAX_TOKEN_COUNT];

    char* meta;
    
    
    
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
            if(commands[numCommands].fileOp == 1)
                commands[numCommands].fileOp = 3;
            else
                commands[numCommands].fileOp = 2;
            token = strtok(NULL, " ");
            continue;
        }
        
        if(commands[numCommands].fileOp == 1){  //this was already set above, set filename
            commands[numCommands].readFile = token;
            token = strtok(NULL, " ");
            continue;
        }
        
        if(commands[numCommands].fileOp == 3 || commands[numCommands].fileOp == 2){  //this was already set above, set filename
            commands[numCommands].writeFile = token;
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

      
      
    int fd[2], in = 0;
    
      
      
    for(int i=0;i<numCommands;i++){
        
    pipe(fd);
        
    //printf(commands[i].command);
        int fileOp = 0;
        char* readFile;
        char* writeFile;
        
            if(commands[i].fileOp!=0){
                fileOp = commands[i].fileOp;
            }
        if(fileOp == 1)
            readFile = commands[i].readFile;
        if(fileOp == 2)
            writeFile = commands[i].writeFile;
        if(fileOp == 3){
            readFile = commands[i].readFile;
            writeFile = commands[i].writeFile;
        }
        runcommand(commands[i].command, &commands[i].args, fileOp, readFile, writeFile, i, numCommands, fd, &in);
        
      
    }

    
  }
  return 0;
}
