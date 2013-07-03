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


