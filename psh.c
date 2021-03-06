/* 
 * psh - A prototype tiny shell program with job control
 * 
 * <Put your name and login ID here>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "util.h"



/* Global variables */
int verbose = 0;            /* if true, print additional output */

extern char **environ;      /* defined in libc */
static char prompt[] = "psh> ";    /* command line prompt (DO NOT CHANGE) */
/* End global variables */


/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);

/* Here are helper routines that we've provided for you */
void usage(void);
void sigquit_handler(int sig);



/*
 * main - The shell's main routine 
 */
int main(int argc, char **argv) 
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
	        break;
        case 'v':             /* emit additional diagnostic info */
            verbose = 1;
	        break;
        case 'p':             /* don't print a prompt */
            emit_prompt = 0;  /* handy for automatic testing */
	        break;
	    default:
            usage();
	    }
    }
    
    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler); 

    /* Execute the shell's read/eval loop */
    while (1) {

	/* Read command line */
	if (emit_prompt) {
	    printf("%s", prompt);
	    fflush(stdout);
	}
	if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
	    app_error("fgets error");
	if (feof(stdin)) { /* End of file (ctrl-d) */
	    fflush(stdout);
	    exit(0);
	}

	/* Evaluate the command line */
	eval(cmdline);
	fflush(stdout);
	fflush(stdout);
    } 

    exit(0); /* control never reaches here */
}
  
/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a built-in command (quit)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return. 
*/
void eval(char *cmdline) 
{   
    /* Keegan driving 
    * Variables to tell us if an input is a command, a background job,
    * the status of a pid, and the pid. The argv variable will hold
    * the input from the user.
    * 
    * Influenced by eval function in B&O pg. 791
    */
    int isCommand, isBG, status; 
    char *argv[MAXARGS];
    pid_t pid; 

    /* Call parseline to read the input from the user store it into
    * argv, and save the return value to isBG. Afterwards send the
    * array of strings to check if the first string is a command. 
    */
    isBG = parseline(cmdline, argv); 
    isCommand = builtin_cmd(argv); 

    /*
    * If the first word of the input was a command, our shell should
    * handle it. Otherwise, we should execute the file at the pathname.
    */
    if (!isCommand) {

        /* Juan driving
        * Create a child process and save the pid.
        */
        pid = fork();

        /* If we are in the child process we should execute the file.
        * Otherwise, we go into the parent process section.
        */
        if(pid == 0) {

            /* In the child process execute the file at the pathname
            * and exit the program with an error message if the
            * execution failed.
            */
            if(execve(argv[0], argv, environ) < 0) {
                printf("%s: is not a command.\n", argv[0]);
                exit(0);
            }

        /* In the parent process check if the first word is a background
        * or foreground job. Here we wait if it's a foreground job.
        */
        } else {
            if (!isBG) {
                waitpid(pid, &status, 0);
            }
        }
    }
    return;
}


/* 
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately. 
 * Return 1 if a builtin command was executed; return 0
 * if the argument passed in is *not* a builtin command.
 */
int builtin_cmd(char **argv) 
{
    /* Keegan driving now
    * If the first word is "quit" then exit program.
    * The strcmp function returns 0 if two strings are equal.
    */
    if (!strcmp(argv[0], "quit")) {
        exit(0); 
    }
    return 0;     /* not a builtin command */
}

/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void) 
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig) 
{
    ssize_t bytes; 
    const int STDOUT = 1; 
    bytes = write(STDOUT, "Terminating after receipt of SIGQUIT signal\n", 45); 
    if(bytes != 45) 
       exit(-999);
    exit(1);
}