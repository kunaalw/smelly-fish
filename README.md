smelly-fish
===========




CS111 (Summer 2013) - Project 1
-------------------------------



Project Team:
 * Kunaal Wadhwa (603826547)
 * Avinash Chandrashekar (303718035)



Lab 1a
------

Notes:

 * First we created tokens by parsing the input stream
   The definition of the token types etc. can be found in read_command.c

 * Next we cleaned up the token stream. This was done by removing extraneous
   newlines, finding malformed commands etc.

 * The tokens were then converted to commands in a separate function. Normal
   precedence ordering was used. For subshells, a subshell was parsed when the
   token stream began with a ( and ended with a ).

 * Redirects were parsed in the simple case.

 * A command tree for each command was returned. The command_stream is an 
   array of pointers to command trees


Limitations
-----------

 * There are some cases of malformed commands that are ignored. It works as
   required for all well-formed commands. 
   !! NOTE: THIS HAS BEEN FIXED FOR MOST (IF NOT ALL) CASES !!
   
 
Lab 1b
------

Notes:

 * Added 2 new files - execute-internals.h and execute-internals.c
   These contain execution functions for individual types of commands and their prototypes in the header
 * Made changes to the makefile to accommodate the above changes
 
Lab 1c
-------
 * Added 2 new files - timetravel.c and timetravel.h
   These contain the timetravel functions
 * These contain the functions which
   Create the dependency pool
   Update Dependencies
 * We use pthreads to create a multithreaded application and we can set the # of threads using a macro
 * Dependency checking
 
 We build a list of files used by ALL commands (n), and count the # of commands (m)
 We now build a 2D array which has m rows and n columns and populate it with the file-command relationship
 We place a 1 where this relationship exists.
 When a command is executed, all entries in its row are zeroed
 If a command is dependent on a previous command then it isn't executed until it is independent
 This design avoids ALL write-write races and we only run into read-write races. This only delays a thread's execution
 and is not fatal to accuracy or health
 
Limitations
-----------
 
 * We parse commands as command flag(absent or present) list of files
   This means something like echo 1 and echo 1 are listed as dependent.
   We could potentially get around this limitation by using a dictionary of commands which are never dependent
   But this has its own drawbacks with regard to corner cases.
   
   We feel like this is a reasonable restriction because it is better to slightly sacrifice speed for accuracy.
   
   
