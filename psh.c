/*
   Pseudo Shell
   File Name:       psh.c

   Authors:         Miguel Cazares
                    Stephen Chan

   Contributor:     Azadeh Razaghian

   Build interface: gcc -o psh psh.c
   Run interface:   ./psh [OPTIONAL ARGUMENT]

   Optional Argument: prompt to be printed

   If no optional argument given, "psh> " will be
   set as the default prompt.   
*/


// required header files

#include<unistd.h>
#include<errno.h>
#include<sys/resource.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<string.h>
#include<sys/wait.h>
#include<signal.h>
#include<fcntl.h>
#include "psh.h"

// Function: init_bproc()
//
// This initializes the background process pid array
// to all 0's
//
// Receives: nothing
void init_bproc()
{
   int i;
   for(i = 0; i < MAXPRC; i++)
   {
      bproc[i] = 0;
   }
}

// Function: cat(int, char*)
//
// This function is our implementation of the cat
// command. Works exactly like the standard bash
// implementation.
//
// Receives: numbers of arguments (files) to the command,
//           and also the file names.
void cat(int numargs, char* argvec[])
{
  int nbytes = 256;
  char buf[nbytes];
  int i;
  int nread;
  int fd;

// If numargs is 1, means no arguments to cat
// Then we read input from keyboard and write to
// stdout.
  if(numargs == 1)
  {
     while((nread = read(STDIN_FILENO, buf, sizeof(buf))) > 0)
        write(STDOUT_FILENO, buf, nread);
  }

  for (i = 1 ; i < numargs ; i++)
  {
     fd = open(argvec[i],O_RDONLY);

     while((nread = read(fd, buf, sizeof(buf))) > 0)
     {
        write(STDOUT_FILENO, buf, nread);
     }

     close(fd);
  }
}

// Function: put_bg(pid_t)
//
// This places a background process id into the
// background process pid array. It looks for the
// first 0 pid in the array (meaning an empty
// spot) and places the background pid there. If
// no spot was found, returns false. Else, it returns
// true.
//
// Receives: background pid to put into bg pid array.
// Returns: bool indicating whether add was successful
//          or not.
bool put_bg(pid_t bpid)
{
   bool placed = false;
   int i;

   for(i = 0; i < MAXPRC; i++)
   {
      if(bproc[i] == 0)
      {
         bproc[i] = bpid;
         placed = true;
         break;
      }
   }

   return placed;
}

// Function: rm_bg(pid_t)
//
// This removes a background pid from the pid array.
// It looks for the background pid given in the pid array.
// If found, removes it from pid array, and returns true.
// Else, it returns false
//
// Receives: background pid to be removed from bg pid array.
// Returns: bool indicating whether remove was successful.
bool rm_bg(pid_t bpid)
{
   bool removed = false;
   int i;

   for(i = 0; i < MAXPRC; i++)
   {
      if(bproc[i] == bpid)
      {
         bproc[i] = 0;
         removed = true;
         break;
      }
   }

   return removed;
}

// Function: kill_all()
//
// This sends a kill signal to all the background process
// that were previously launched. This effectively goes
// through the entire pid array and any non-zero pid found
// will be sent a kill signal. A message will be printed 
// indicating process was killed (if successful). The function
// also zeroes out the pid in that spot.
//
// Receives: nothing
// Returns: void
void kill_all()
{
   int ret;
   int i;

   for(i = 0; i < MAXPRC; i++)
   {
      if(bproc[i] != 0)
      {
         ret = kill(bproc[i], SIGKILL);
         if(ret != 0)  // if kill failed, print out error message
         {
            printf("Kill Failed for PID: %d\n", bproc[i]);
         }
         else  // else set the background pid array element to zero
         {
            printf("Process with PID: %d killed\n", bproc[i]);
            bproc[i] = 0;
         }
      }
   }
}

// Function: remove_newline(char*)
//
// This removes the newline character at the end of a string.
// This is useful to get fgets to act like gets (but in a safe manner).
//
// Receives: pointer to a c-style string
// Returns: pointer to modified c-style string  
char* remove_newline(char* string)
{
    int length = strlen(string);

    if (length > 0 && string[length-1] == '\n')  // if there's a newline
        string[length-1] = '\0';                 // then, truncate the string

    return string;
}
        
// Function: changelnToSpace()
// Convert user input to NULL-terminated argument list
// This receives what was entered at the command prompt.
// Parses through the user command and enters null terminators
// where needed. This basically modifies the user command so that
// we can pass it to execvp. This receives a one dimensional array (ucommand)
// of characters and copies the array into a two dimensional array (argv).
// Each column of the array is a command argument. Upon return, the function
// returns how many arguments were in the one dimensional array.
//
// Receives: char pointer (ucommand) -- user command
//           argv[]
// Returns: integer count of the number of arguments. 
int  parseCommand(char *ucommand, char **argv)
{
    int count = 0;
	                             // while we are not at EOL and process is in foreground
    while (*ucommand != '\0')
    {                          
      while (*ucommand == ' ' || *ucommand == '\t' || *ucommand == '\n')
      {
        *ucommand++ = '\0';      // " " = '\0'    
      }

      *argv++ = ucommand;        // save the argument position     

      while (*ucommand != '\0' && *ucommand != ' ' &&  *ucommand != '\t' && *ucommand != '\n')
      {
          ucommand++;           // skip the argument until ...    
      }

      if(*ucommand == ' ')
      {
         count++;
      }
    }
    *argv = '\0';              /* NULL Terminator:  end of arguments  */
    return count;
}

// Function: print_bg_msg(int, pid_t)
//
// This prints a message stating a background process finished
// and specifies whether the process finished normally or abnormally
// according to its exit status.
//
// Precondition: The background process must have already finished,
//               this function only prints out the exit messages
// Receives: the status from the waitpid call and background pid
// Returns: void 
void print_bg_msg(int status, pid_t back_pid)
{
   printf("\nBackground process finished with PID: %d\n", back_pid);
   
   if(WIFEXITED(status))
      printf("Background Process exited normally with exit code: %d\n", WEXITSTATUS(status));
   else if(WIFSIGNALED(status))
      printf("Background Process terminated by signal: %d\n", WTERMSIG(status));
   else if(WIFSTOPPED(status))
      printf("Background Process stopped by signal: %d\n", WSTOPSIG(status));
   else
      printf("Status or exit code could not be interpreted\n");
}

// Function: update_status()
//
// poll for exited child processes, remove from background process
// list -- if child processe(s) have exited
void update_status()
{
   int i;
   int status;
   
   for(i = 0; i < MAXPRC; i++)
   {
      if(bproc[i] != 0)
      {
         if(waitpid(bproc[i], &status, WNOHANG) == bproc[i])
         {
            // we must sleep before printing out any messages so
            // that they don't interfere with current output
            sleep(1);
            print_bg_msg(status, bproc[i]);
            rm_bg(bproc[i]);
         }
      }
   }
}
            
// Function: prompt(char, char*)
//
// This is the command prompt that prompts
// for a command and saves user input
// If user only presses enter or return, then
// the user is prompted again until a command
// of at least one character is entered.
void prompt(char cmd[], char* cprompt)
{
  do
  {
    printf("%s", cprompt);
    cmd = fgets(cmd, CMDSIZE, stdin);
    cmd = remove_newline(cmd);
    update_status();
  } while(*cmd == '\0'|| *cmd == '\n');
}

// This signal handler is useful for handling the user
// entering CTRL-C while a foreground process is running
void sighandler(int sig)
{
   fg = false;
   printf("Killed foreground process with PID: %d\n", fg_pid); 
   kill(fg_pid, SIGKILL);
}

int main (int argc, char** argv)
{       
/* DECLARATION SECTION */
  int ret;    		       // return value for error-checking macro
  char ucommand[CMDSIZE];      // character array for user command
  char* cmdprompt;             // char pointer for command prompting
 
  int status;                  // child process exit status
  int resultexec;              // command executopm results

  pid_t pid;                   // necesary to use pid system calls
  struct sigaction newaction;  // register signal handler
  int numargs;                 // number of arguments to user-typed command
  int i;                       // index used in for-loops
  bool prev_cmd;               // boolean to determine if a command was entered
                               // while a foreground process was running
							   
/* INITIALIZATION SECTION */
  main_pid = getpid();
  shell_pid = getppid();

  newaction.sa_handler = SIG_IGN;
  sigaction(SIGINT, &newaction, 0);

// Initialize background pid array to all zeros (means no bg processes running)
  init_bproc();

// Set default command prompt if arg number is 1,
// otherwise set custom command prompt, if arg number is 2
  if(argc == 1)
  {
     cmdprompt = "psh> ";
  }
  else if(argc == 2)
  {
     cmdprompt = argv[1];
     strcat(cmdprompt, " ");
  }
  else
  {
     printf("Too many arguments..\nTerminating..\n");
     return -1;
  }

/* START MAIN LOOP (RUNS UNTIL USER TYPES "EXIT") */
  while (1) 
  {
    // First we make sure to set the signal handler to ignore CTRL-C typed
    // at the keyboard (until a foreground process runs, then we switch)
    newaction.sa_handler = SIG_IGN;
    sigaction(SIGINT, &newaction, 0);

    // Here we want to make sure and check the pid array to see if any of the
    // previously launched processes have finished, if so, we print out exit
    // status messages and update our pid array to make room for newer
    // background processes
    update_status();

    prompt(ucommand, cmdprompt);

    // We want to parse the command (will be stored in argv).
    // We also get the number of arguments the user entered and store it.
    numargs = parseCommand(ucommand , argv);

    // Here we are checking to see if the last string in the command array
    // is an '&' character, which means we should handle this process as a
    // background process. We also delete the '&' character if we find it
    // since this will be passed to the execvp function call
    if(strcmp(argv[numargs],"&") == 0)
    {
       bg = true;
       fg = false;
       argv[numargs] = NULL; // if the last argument is an '&' character, replace with a space
    }
    else
    {
       bg = false;
       fg = true;
    }

    // If user types exit, exit the program. 
    // Before exiting, the shell kills all background processes
    if (strcmp(argv[0], "exit") == 0)
    {
      printf("Pseudo Shell Now Terminating...\n");
      kill_all();
      exit(0);
    }
    
    /* START FORKING */
    pid = fork();

    if (pid < 0 )
    { 
       printf("Error creating child process.\n");
       exit(1);
    }
    else if (pid == 0) // pid == 0 means we are in child
    {
    // If user types kill, we make sure and remove
    // the background process from our data structure of pids
	// Note: the kill command in the loop is needed because the execvp call
    //       with "kill pid0 pid1... etc" was not actually killing
    //       the background processes
    if(numargs >= 1)
    {
       if(strcmp(argv[0], "kill") == 0)
       {
          for(i = 1; i <= MAXPRC; i++)
          {
             if(atoi(argv[i]) == main_pid || atoi(argv[i]) == shell_pid)
             {
                printf("\nERROR: PID passed to KILL command is either this program\n");
                printf("       or it is the shell running this program...\n");
                printf("       If you want to exit this program (pseudo shell), enter \"exit\"\n\n");
                exit(0);
             }

             rm_bg(atoi(argv[i]));
             kill(atoi(argv[i]), SIGKILL);
          }
       }
    } 
	
    // If user types cat command, we run our implementation of cat
    // and then exit, we do not let execvp handle the command
    if(strcmp(argv[0], "cat") == 0)
    {
       cat(numargs + 1, argv);
       exit(0);
    }

   // Here we let the macro check for any abnormal return messages
   // of the execvp system call
    CHECK(execvp(*argv, argv));
    }
    else  // else we are in parent (which is the shell itself)
    { 
       if(fg) // process is a foreground
       {
          // save the foreground pid in variable fg_pid
          fg_pid = pid;
          // if we are a foreground process, then we must use the custom
          // signal handler function we wrote, which kills the foreground
          // process but does not kill the shell per the instructions

          newaction.sa_handler = sighandler;
          sigaction(SIGINT, &newaction, 0);

         while(wait(&status) != fg_pid);

         // show exit statuses for foreground processes          
       if(WIFEXITED(status))
          {
             printf("Foreground process exited normally with  status: %d\n", WEXITSTATUS(status));
          }
          else if(WIFSIGNALED(status))
          {
             printf("Foreground process was killed by signal: %d\n", WTERMSIG(status));
          }
          else if(WIFSTOPPED(status))
          {
             printf("Foreground process was stopped with signal: %d\n", WSTOPSIG(status));
          }
  
       }
       else // process is a background
       {
          bool added;
 
          bg_pid = pid;
          added = put_bg(bg_pid);

          // in case there are already maximum number of background proceses
          // running, then we want to make sure and kill the child we just forked
          // and print out an error message
          if(!added)
          {
             kill(bg_pid, SIGKILL);
             printf("ERROR: Could not launch new background process\n");
             printf("       Exceeded Maximum Allowable Background Processes of %d.\n", MAXPRC);
          }
          else
          {
             // if the background process finishes at this point
             // then go ahead and print out its exit status
             if(waitpid(bg_pid, &status, WNOHANG) == bg_pid)
             {
                print_bg_msg(status, bg_pid);
                rm_bg(bg_pid);
             }
             // else, the background process hasn't finished
             // so we only print out message saying it was launched
             else
             {
                printf("Background process launched with PID: %d\n", bg_pid);
             }
          }
       }
    } //end else (parent block of code)
  }   // end while loop (main loop)
}    
