#include "../include/msgs.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <pwd.h>
#include <limits.h>
#include <sys/stat.h>
#define MAX_HISTORY_SIZE 10

//volatile sig_atomic_t sigint_received = 0;

void sigint_handler(int signo){
  if(signo == SIGINT){
    //signal(SIGINT, handler_function);
    write(STDOUT_FILENO, "\n", 1);
    write(STDOUT_FILENO, FORMAT_MSG("cd", CD_HELP_MSG), strlen(FORMAT_MSG("cd", CD_HELP_MSG)));
    write(STDOUT_FILENO, FORMAT_MSG("exit", EXIT_HELP_MSG), strlen(FORMAT_MSG("exit", EXIT_HELP_MSG)));
    write(STDOUT_FILENO, FORMAT_MSG("pwd", PWD_HELP_MSG), strlen(FORMAT_MSG("pwd",PWD_HELP_MSG)));
    write(STDOUT_FILENO, FORMAT_MSG("help", HELP_HELP_MSG), strlen(FORMAT_MSG("help", HELP_HELP_MSG)));
    write(STDOUT_FILENO, FORMAT_MSG("history",HISTORY_HELP_MSG), strlen(FORMAT_MSG("history", HISTORY_HELP_MSG)));   
    
    char cwd[256];
    if(getcwd(cwd, sizeof(cwd))!=NULL){
      write(STDOUT_FILENO, cwd, strlen(cwd));
      write(STDOUT_FILENO, "$", 1);
    }
    fflush(stdout);
    //sigint_received = 1;
  }
}

typedef struct {
  int number;
  char command[256];
} CommandHistory;

CommandHistory history [MAX_HISTORY_SIZE];
int commandCount = 0;

void addToHistory(const char* commandText){
  if(commandCount < MAX_HISTORY_SIZE){
    history[commandCount].number = commandCount;
    strcpy(history[commandCount].command, commandText);
    commandCount++;
  }
  else{
    for(int i=0; i<MAX_HISTORY_SIZE-1; i++){
      history[i] = history[i+1];
      history[i].number = i;
    }
    strcpy(history[MAX_HISTORY_SIZE - 1].command, commandText);
  }
}

void displayHistory() {

  /*int maxNumLength = 1;
  for(int i=0; i<commandCount; i++){
    int numLength = snprintf(NULL, 0, "%d", history[i].number);
    if(numLength > maxNumLength){
      maxNumLength = numLength;
    }
  }*/

  for(int i=commandCount-1; i>=0; i--){
    char historyMsg[512];
    //int numLength = snprintf(NULL, 0, "%d", history[i].number);
    //int padding = maxNumLength - numLength;
    snprintf(historyMsg, sizeof(historyMsg), "%d\t%s\n", history[i].number, history[i].command);
    write(STDOUT_FILENO, historyMsg, strlen(historyMsg));
  }
}

int main(){

  signal(SIGINT, sigint_handler);
  /*struct sigaction sa;
  sa.sa_handler = sigint_handler;
  sigaction(SIGINT, &sa, NULL);
*/
  char input[256];
  char cwd[256];

  while(1){

    /*if(sigint_received){
      write(STDOUT_FILENO, "\n", 1);
      write(STDOUT_FILENO, FORMAT_MSG("cd", CD_HELP_MSG), strlen(FORMAT_MSG("cd", CD_HELP_MSG)));
      write(STDOUT_FILENO, FORMAT_MSG("exit", EXIT_HELP_MSG), strlen(FORMAT_MSG("exit", EXIT_HELP_MSG)));
      write(STDOUT_FILENO, FORMAT_MSG("pwd", PWD_HELP_MSG), strlen(FORMAT_MSG("pwd",PWD_HELP_MSG)));
      write(STDOUT_FILENO, FORMAT_MSG("help", HELP_HELP_MSG), strlen(FORMAT_MSG("help", HELP_HELP_MSG)));
      write(STDOUT_FILENO, FORMAT_MSG("history",HISTORY_HELP_MSG), strlen(FORMAT_MSG("history", HISTORY_HELP_MSG)));   
      sigint_received = 0;
      char cwd[256];
      if(getcwd(cwd, sizeof(cwd))!=NULL){
        write(STDOUT_FILENO, cwd, strlen(cwd));
        write(STDOUT_FILENO, "$", 1);
      }
    }

    else{*/

    if(getcwd(cwd, sizeof(cwd)) == NULL) {
      write(STDERR_FILENO, FORMAT_MSG("shell", GETCWD_ERROR_MSG), strlen(FORMAT_MSG("shell", GETCWD_ERROR_MSG)));
      exit(1);
    }

    write(STDOUT_FILENO, cwd, strlen(cwd));
    write(STDOUT_FILENO, "$", 2);

    ssize_t n = read(STDIN_FILENO, input, sizeof(input));

    if(n == -1){
      write(STDERR_FILENO, FORMAT_MSG("shell", READ_ERROR_MSG), strlen(FORMAT_MSG("shell", READ_ERROR_MSG)));
      exit(1);
    }

    input[strlen(input) - 1] = '\0';

    //handling background
    int background = 0;
    if(input[strlen(input) - 1] == '&'){
      background = 1;
      input[strlen(input) - 1] = '\0';
    }
    
    char *args[32];
    int total_arg = 0;
    char *token = strtok(input, " ");
    
    while(token != NULL){
      args[total_arg] = token;
      token = strtok(NULL, " ");
      total_arg++;
    }
    args[total_arg] = NULL;
    
    //char previousDirectory[512];

    if(total_arg > 0){

     /* char fullCommand[256];
      strcpy(fullCommand, args[0]);
      for(int i=1; i<total_arg; i++){
        strcat(fullCommand, " ");
        strcat(fullCommand, args[i]);
      }
      if(background == 1){
        strcat(fullCommand, "&");
      }
      addToHistory(fullCommand);
      
      if (strcmp(args[0], "history") == 0){
          if(total_arg > 1){
            write(STDERR_FILENO, FORMAT_MSG("history", TMA_MSG), strlen(FORMAT_MSG("history", TMA_MSG)));
          }
          else{
            displayHistory();
          }
      }
 
      else*/ if(strcmp(args[0], "exit") == 0){
        if(total_arg > 1){
          write(STDERR_FILENO, FORMAT_MSG("exit", TMA_MSG), strlen(FORMAT_MSG("exit", TMA_MSG)));
          write(STDERR_FILENO, "\n", 0);
        }

        else{
          exit(0);
        }
      }
      else if(strcmp(args[0], "pwd") == 0){
        
        if(total_arg > 1){
          write(STDERR_FILENO, FORMAT_MSG("pwd", TMA_MSG), strlen(FORMAT_MSG("pwd", TMA_MSG)));
        }

        else{

          char* curr_dir = getcwd(NULL, 0);
          if(curr_dir == NULL){
            write(STDERR_FILENO, FORMAT_MSG("pwd", GETCWD_ERROR_MSG), strlen(FORMAT_MSG("pwd", GETCWD_ERROR_MSG)));  
          }
          else{
            write(STDOUT_FILENO, curr_dir, strlen(curr_dir));
            write(STDOUT_FILENO, "\n", 1);
            free(curr_dir);
          }
        }
      }
      else if(strcmp(args[0], "cd") == 0){
        

        /*  if(total_arg == 1){
          struct passwd *pw = getpwuid(getuid());
          char* homeDir = pw->pw_dir;
          if(chdir(homeDir)!=0){
            write(STDERR_FILENO, FORMAT_MSG("cd", CHDIR_ERROR_MSG), strlen(FORMAT_MSG("cd", CHDIR_ERROR_MSG)));
          }
          else{
            strcpy(previousDirectory, homeDir);
          }
        }

        else if (total_arg == 2){
          if(strcmp(args[1], "-") == 0){
            if(chdir(previousDirectory)!=0){
              write(STDERR_FILENO, FORMAT_MSG("cd", CHDIR_ERROR_MSG), strlen(FORMAT_MSG("cd", CHDIR_ERROR_MSG)));
            }
          }
          else if(args[1][0] == '~'){
            struct passwd *pw = getpwuid(getuid());
            char *homeDir = pw->pw_dir;
            char fullPath[256];
            snprintf(fullPath, sizeof(fullPath), "%s%s", homeDir, args[1]+1);
            if(chdir(fullPath)!=0){
              write(STDERR_FILENO, FORMAT_MSG("cd", CHDIR_ERROR_MSG), strlen(FORMAT_MSG("cd", CHDIR_ERROR_MSG)));
            }
          }
          else{
            if(chdir(args[1])!=0){
              write(STDERR_FILENO, FORMAT_MSG("cd", CHDIR_ERROR_MSG), strlen(FORMAT_MSG("cd", CHDIR_ERROR_MSG)));
            }
          }
        }*/

        if(total_arg>2){
          
          write(STDERR_FILENO, FORMAT_MSG("cd", TMA_MSG), strlen(FORMAT_MSG("cd", TMA_MSG)));
        }
        
        else{
          char* change_to = args[1];
        
          if(change_to == NULL || strcmp(change_to, "~") == 0){
            struct passwd *pw = getpwuid(getuid());
            change_to = pw->pw_dir;  
          }
          
          else if(strcmp(change_to, "-") == 0){
            change_to = getenv("OLDPWD");
            if(change_to == NULL){
              write(STDERR_FILENO, "cd: OLDPWD not set\n", 18);
            }
          }

          if(chdir(change_to) == -1){
            write(STDERR_FILENO, FORMAT_MSG("cd", CHDIR_ERROR_MSG), strlen(FORMAT_MSG("cd", CHDIR_ERROR_MSG)));
            
          }
          /*char prev_dir[256];
          if(getcwd(prev_dir, sizeof(prev_dir)) != NULL){
             setenv("OLDPWD", prev_dir, 1);
          }*/
          else{
            /*char current_dir[4096];
            if(getcwd(current_dir, sizeof(current_dir))!=NULL){
              if(setenv("PWD", current_dir, 1) == -1){
                write(STDERR_FILENO, "cd: error updating PWD environment variable\n", 44);
              }
            }
            else{*/
              write(STDERR_FILENO, FORMAT_MSG("cd", GETCWD_ERROR_MSG), strlen(FORMAT_MSG("cd", GETCWD_ERROR_MSG)));
          }
        } 
      }
               
      else if (strcmp(args[0], "help") == 0){
        if(total_arg > 2){
          write(STDERR_FILENO, FORMAT_MSG("help", TMA_MSG), strlen(FORMAT_MSG("help", TMA_MSG)));
        }
        else{
          if(total_arg == 2){
            if(strcmp(args[1], "cd") == 0){
              write(STDOUT_FILENO, FORMAT_MSG("cd", CD_HELP_MSG), strlen(FORMAT_MSG("cd", CD_HELP_MSG)));
            }
            else if(strcmp(args[1], "exit") == 0){
              write(STDOUT_FILENO, FORMAT_MSG("exit", EXIT_HELP_MSG), strlen(FORMAT_MSG("exit", EXIT_HELP_MSG)));
            }
            else if(strcmp(args[1], "pwd") == 0){
              write(STDOUT_FILENO, FORMAT_MSG("pwd", PWD_HELP_MSG), strlen(FORMAT_MSG("pwd",PWD_HELP_MSG)));
            }
            else if (strcmp(args[1], "help") == 0){
              write(STDOUT_FILENO, FORMAT_MSG("help", HELP_HELP_MSG), strlen(FORMAT_MSG("help", HELP_HELP_MSG)));
            }
            else{
              write(STDOUT_FILENO, args[1], strlen(args[1]));
              write(STDOUT_FILENO, FORMAT_MSG("", EXTERN_HELP_MSG), strlen(FORMAT_MSG("", EXTERN_HELP_MSG)));
            }
          }
          else{
          write(STDOUT_FILENO, FORMAT_MSG("cd", CD_HELP_MSG), strlen(FORMAT_MSG("cd", CD_HELP_MSG)));
          write(STDOUT_FILENO, FORMAT_MSG("exit", EXIT_HELP_MSG), strlen(FORMAT_MSG("exit", EXIT_HELP_MSG)));
          write(STDOUT_FILENO, FORMAT_MSG("pwd", PWD_HELP_MSG), strlen(FORMAT_MSG("pwd",PWD_HELP_MSG)));
          write(STDOUT_FILENO, FORMAT_MSG("help", HELP_HELP_MSG), strlen(FORMAT_MSG("help", HELP_HELP_MSG)));
          write(STDOUT_FILENO, FORMAT_MSG("history",HISTORY_HELP_MSG), strlen(FORMAT_MSG("history", HISTORY_HELP_MSG)));  
          }
        }
      }
/*      else if(strcmp(args[0], "history") == 0){
        int start;
        if(historyCount>=MAX_HISTORY_SIZE){
          start = historyCount - MAX_HISTORY_SIZE;
        }
        else{
          start = 0;
        }

        for(int i = start; i<historyCount; i++){
          int index = i%MAX_HISTORY_SIZE;
          char historyLine[256];
          if(history[index].background){
            snprintf(historyLine, sizeof(historyLine), "%d (bg): %s", history[index].number, history[index].text);
          }
          else{
            snprintf(historyLine, sizeof(historyLine), "%d %s", history[index].number, history[index].text);
          }
          write(STDOUT_FILENO, historyLine, strlen(historyLine));
          write(STDOUT_FILENO, "\n", 1);
        }
      }*/
     /* else{
        write(STDERR_FILENO, FORMAT_MSG("shell", EXEC_ERROR_MSG), strlen(FORMAT_MSG("shell", EXEC_ERROR_MSG)));
        //fprintf(stderr, FORMAT_MSG("shell", EXEC_ERROR_MSG));
        exit(1);
      }*/
      else{
    
        pid_t pid = fork();
    
        if(pid < 0){
          write(STDERR_FILENO, FORMAT_MSG("shell", FORK_ERROR_MSG), strlen(FORMAT_MSG("shell", FORK_ERROR_MSG)));
          exit(1);
        }

        else if (pid == 0){
      
          if(execvp(args[0], args) == -1){
            write(STDERR_FILENO, FORMAT_MSG("shell", EXEC_ERROR_MSG), strlen(FORMAT_MSG("shell", EXEC_ERROR_MSG)));
            //write(STDERR_FILENO, "\n", 1);
            exit(1);
          }
        } 

        else{
          if (background == 0){
            int status;
            if(waitpid(pid, &status, 0) == -1){
              write(STDERR_FILENO, FORMAT_MSG("shell", WAIT_ERROR_MSG), strlen(FORMAT_MSG("shell", WAIT_ERROR_MSG)));
              exit(1);
            }
          }
          else{
            int status;
            while(waitpid(-1, &status, WNOHANG)>0);
          }
        }
      }
    }
  }
}
