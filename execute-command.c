// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"
#include "execute-internals.h"
#include <error.h>
#include <stdio.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

int
command_status (command_t c)
{
  return c->status;
}

void execute_command (command_t c, int time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
if (time_travel == 0)
no_tt(c);

else
printf("You shouldn't get here");

/*
  error (1, 0, "you shouldn't really get here");
  */
}

void no_tt(command_t c)
{
switch(c->type)
	{
    
    case OR_COMMAND:
      execute_or(c);
      break;
	  
	case AND_COMMAND:
      execute_and(c);
      break;
	  
    case PIPE_COMMAND:
      execute_pipe(c);
      break;
	  
    case SIMPLE_COMMAND:
      execute_simple(c);
      break;
	  
	case SEQUENCE_COMMAND:
      execute_sequence(c);
    break;
	
    case SUBSHELL_COMMAND:
      execute_subshell(c);
      break;
	  
    default:
      error(1, 0, "Command Type Incorrect - Command does not belong to any supported type");
	}
	
}
