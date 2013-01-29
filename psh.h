/*
 Pseudo Shell
 File Name:       psh.h
 
 Authors:        Miguel Cazares
                 Stephen Chan
 
 Contributor:    Azadeh Razaghian
 
 Build interface: gcc -o psh psh.c
 Run interface:   ./psh [OPTIONAL ARGUMENT]
 
 Optional Argument: prompt to be printed
 
 If no optional argument given, "psh> " will be
 set as the default prompt.
 */

#define CMDSIZE 300    // Maximum # of characters allowed for a command
#define MAXPRC 2      // Maximum # of processes allowed to run in background

// Error check macro
#define CHECK(CALL)\
if((CALL) < 0 ) {\
perror(#CALL);\
printf("\n");\
exit(1);\
}\

typedef int bool; // defines 'bool' for C
#define true 1
#define false 0

/* VARIABLE DECLARATION & INITIALIZATION */
bool bg;  	           // background flag
bool fg;	           // foregrund flag

pid_t fg_pid;	       // process id of foreground process
pid_t bg_pid;          // process id of background process

pid_t main_pid;        // process id of this program
pid_t shell_pid;       // process id of the shell running this program

pid_t bproc[MAXPRC];   // background pid array

// Function: init_bproc()
//
// This initializes the background process pid array
// to all 0's
//
// Receives: nothing
void init_bproc();

// Function: cat(int, char*)
//
// This function is our implementation of the cat
// command. Works exactly like the standard bash
// implementation.
//
// Receives: numbers of arguments (files) to the command,
//           and also the file names.
void cat(int numargs, char* argvec[]);

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
bool put_bg(pid_t bpid);

// Function: rm_bg(pid_t)
//
// This removes a background pid from the pid array.
// It looks for the background pid given in the pid array.
// If found, removes it from pid array, and returns true.
// Else, it returns false
//
// Receives: background pid to be removed from bg pid array.
// Returns: bool indicating whether remove was successful.
bool rm_bg(pid_t bpid);

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
void kill_all();

// Function: remove_newline(char*)
//
// This removes the newline character at the end of a string.
// This is useful to get fgets to act like gets (but in a safe manner).
//
// Receives: pointer to a c-style string
// Returns: pointer to modified c-style string
char* remove_newline(char* string);

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
int  parseCommand(char *ucommand, char **argv);

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
void print_bg_msg(int status, pid_t back_pid);

// Function: update_status()
//
// poll for exited child processes, remove from background process
// list -- if child processe(s) have exited
void update_status();

// Function: prompt(char, char*)
//
// This is the command prompt that prompts
// for a command and saves user input
// If user only presses enter or return, then
// the user is prompted again until a command
// of at least one character is entered.
void prompt(char cmd[], char* cprompt);

// This signal handler is useful for handling the user
// entering CTRL-C while a foreground process is running
void sighandler(int sig);
