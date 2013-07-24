#include "command.h"
#include "command-internals.h"
#include "execute-internals.h"
#include <error.h>
#include <stdio.h>
#include <string.h>
/*
Functions/vars used
pid_t
waitpid 
_exit
fork
execvp
exit
*/

/*
We use _exit to exit the child function as opposed to exit

http://stackoverflow.com/questions/5422831/what-is-the-difference-between-using-exit-exit-in-a-conventional-linux-fo

has an answer explaining why

You should use _exit (or its synonym _Exit) to abort the child program when the exec fails, because in this situation, the child process may interfere with the parent process' external data (files) by calling its atexit handlers, calling its signal handlers, and/or flushing buffers.

This is key in the pipe function as we shall see

*/

#include <sys/wait.h> 
#include <unistd.h>  
#include <sys/types.h>  
#include <stdlib.h>  
#include <error.h>
#include <errno.h>
#include "alloc.h"
#include <fcntl.h>			
#include <sys/stat.h>


/*
void execute_pipe (command_t c)
Functions used:
dup2:
dup2() makes newfd be the copy of oldfd, closing newfd first if necessary, but note the following:

*
    If oldfd is not a valid file descriptor, then the call fails, and newfd is not closed. 
*
    If oldfd is a valid file descriptor, and newfd has the same value as oldfd, then dup2() does nothing, and returns newfd.
References: 
http://linux.die.net/man/2/dup2
http://stackoverflow.com/questions/1720535/practical-examples-use-dup-or-dup2
http://stackoverflow.com/questions/11635219/dup2-dup-why-would-i-need-to-duplicate-a-file-descriptor
*/
/*
void execute_pipe (command_t c)

Fork process
depending on how fork takes place copy file descriptor

*/
void execute_pipe (command_t c)
{
  int status;

  pid_t first_pid;
  pid_t return_pid;
  pid_t second_pid;

  int buffer[2];

//Fork creation is nondeterministic
// pid = fork();
// pid ==
// -1 -> Error
//  0  -> Child
// >0 -> Parent  
//http://www.tldp.org/LDP/lpg/node11.html  
  if ( pipe(buffer) == -1 ) 
      error (1, errno, "pipe creation error");
  
  first_pid = fork();
  if( first_pid > 0 ) //parent
  {
    second_pid = fork();
    if( second_pid > 0 ) //No need for pipe in parent
    {
      close(buffer[0]);
      close(buffer[1]);
      return_pid = waitpid(-1, &status, 0); // wait for process completion
      if( return_pid == first_pid )
      {
        c->status = status;
        waitpid(second_pid, &status, 0);
        return;
      }
      else if(return_pid == second_pid)
      {
        waitpid(first_pid, &status, 0);
        c->status = status;
        return;
      }
    }
// 2nd is child, but first part of the pipe
// second_pid -> child process	
    else if( second_pid == 0 ) 
    {
      close(buffer[0]);
      if( dup2(buffer[1], 1) == -1 )
        error (1, errno,  "could not duplicate file descriptor - error in dup2");
      no_tt(c->u.command[0]);
      _exit(c->u.command[0]->status);
	  //_exit instead of exit
    }
    else
      error(1, 0, "fork error");
  }
//1st is child, but second part of the pipe
//first_pid -> child process
  else if( first_pid == 0)
  {
    close(buffer[1]);
      if( dup2(buffer[0], 0)== -1 )
        error (1, errno,  "ould not duplicate file descriptor - error in dup2");
      no_tt(c->u.command[1]);
      _exit(c->u.command[1]->status);
  }
  else
    error(1, 0, "fork failure");
}


/*
A&&B

Execute A
	If true execute B
		If B is true, return true
	Return false
*/
void execute_and(command_t c)
{
  no_tt(c->u.command[0]);
	
  if(c->u.command[0]->status == 0) //If the first succeeds, do second
  {
no_tt(c->u.command[1]);
c->status = c->u.command[1]->status;
  }
  else
c->status = c->u.command[0]->status;
}

/*
A||B
Execute A
	If false
		Execute B
			If true
				return true
			If false
				return false
	If true
		return true
*/

void execute_or(command_t c)
{
  no_tt(c->u.command[0]);
  if(c->u.command[0]->status != 0)  // If the first fails, execute second
  {
    no_tt(c->u.command[1]);
    c->status = c->u.command[1]->status;
  }
  else
    c->status = c->u.command[0]->status;
}

/*
This is where simple commands get executed

execvp(c->u.word[0], c->u.word );
*/
void execute_simple(command_t c)
{
	int status;
	pid_t pid = fork();

	if(pid == 0) //child thread
	{
		filesetup(c);
		
// Possibly check for edge cases here?

//What could they be - so we do not have to execute
//this might speed up the program
//What could they be - so we do not have to execute
//this might speed up the program
//jank test
//thanks for telling me about that word
//assign hello to the second word and run it
//NULL terminated 


/*
c->u.word[0]="expr";
c->u.word[1]="3";
c->u.word[2]="+";
c->u.word[3]="3";
c->u.word[4]=NULL;
*/
 //       printf( " Word is: %s\n", c->u.word[0] );
   const char s[2] = " ";
   char* temp = (char*) checked_malloc(sizeof(char)*(strlen(c->u.word[0])+1));
   //char temp[100];
   char *token;
   /*copy the word*/
   strcpy (temp, c->u.word[0]);
  // printf( " Temp is: %s\n", temp );
   /* get the first token */
   token = strtok(c->u.word[0], s);
   int counter = 0;
   /* walk through other tokens */
   while( token != NULL )
   {
//      printf( "Token is: %s\n", token );
          c->u.word[counter]=token;
 //     printf( "c->u.word[counter] is %s\n", c->u.word[counter] );  
      token = strtok(NULL, s); 
counter++;
   }
int i=0;
c->u.word[counter]=NULL;
/*
for(i=0;i<counter+1;i++)
{
      printf( "c->u.word[%d] is %s\n",i, c->u.word[i] ); 
} 
 */
/*
  char *execArgs[] = { "expr", "3 + 4", NULL };
  execvp(execArgs[0], execArgs);
*/

		execvp(c->u.word[0], c->u.word ); //execution!
		printf("Word is: %s\n", c->u.word[0]);
		printf("Word is: %s\n", c->u.word[1]);
		error(1, 0, "simple command invalid");
	}
	
	else if(pid > 0) // Executing parent process 
	{
		waitpid(pid, &status, 0);//wait for child and then store status
		c->status = status;
	}
	else
		error(1, 0, "fork error");
}

/*
void execute_subshell(command_t c)
This is fairly trivial - we need to setup the file if there is a redirect, and then recursively call no_tt
*/

void execute_subshell(command_t c)
{
filesetup(c);
no_tt(c->u.subshell_command);
_exit(c->u.subshell_command->status);
}
void execute_sequence(command_t c)
{
  int status;

  pid_t pid = fork();

  if(pid == 0) //child
  {
    pid = fork(); //fork another child - too many children!

    if( pid > 0)
    {
      waitpid(pid, &status, 0);

      no_tt(c->u.command[1]);

      _exit(c->u.command[1]->status); //_exit for child - V.Imp
    }
    else if( pid == 0) // grandchild or original process, child of child
    {
      
	  no_tt(c->u.command[0]);
	  
      _exit(c->u.command[0]->status); //_exit()

	  }
    else
      error(1, 0, "Fork failure");
  }
    else if(pid > 0) //parent
  {
    waitpid(pid, &status, 0);
    c->status = status;
  }
  
  else
    error(1, 0, "Fork Failure");
}

/*
Opens files for reading and writing

This takes care of redirect operations

http://linux.die.net/man/3/open

*/
void filesetup (command_t c)
{
if(c->input != NULL)
	{
		int fd_in = open(c->input, O_RDWR); //open with read/write permissions
		if( fd_in < 0)
			error(1, 0, "Read error: %s", c->input);

		if( dup2(fd_in, 0) < 0)
			error(1, 0, "dup2 input error");

		if( close(fd_in) < 0)
			error(1, 0, "input file close error");
	}

//We need to read and write to some output
//We need to create this in case it doesn't already exist
	if(c->output != NULL)
	{
	/*
	Usage
	fd = open(filename, , mode); 
	*/
		int fd_out = open(c->output, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	//I am not sure if this is correct, maybe it needs more or less?
	//I just threw in a bunch of modes here
	//Seems ok
	/*
O_CREAT
If the file exists, this flag has no effect except as noted under O_EXCL below. Otherwise, the file shall be created
	
O_TRUNC
If the file exists and is a regular file, and the file is successfully opened O_RDWR or O_WRONLY, its length shall be truncated to 0, and the mode and owner shall be unchanged. It shall have no effect on FIFO special files or terminal device files. Its effect on other file types is implementation-defined. The result of using O_TRUNC with O_RDONLY is undefined.

If O_TRUNC is set and the file did previously exist, upon successful completion, open() shall mark for update the st_ctime and st_mtime fields of the file.

	
	S_IWGRP/ S_IWGRP
write/read permission, group
	S_IWUSR / S_IRUSR
write/read permission, owner

	
	*/
		if( fd_out < 0)
			error(1, 0, "output file read error: %s", c->output);

		if( close(fd_out) < 0)
			error(1, 0, "output file close error");
	}
}
