//For dedug purposes - blank for now
void dprint(command_t);

/* 
Command Execution Functions for each case
AND      - Implemented
OR       - Implemented
PIPE     - Implemented
SIMPLE   - Implemented
Redirect - Partially Implemented - does not work with subshells
Subshell - Not Implemented
Sequence - Not Implemented
 */
/*
A|B
Execute A and B, pipe A's output -> B's input
Return after B finishes, give exit status of B
*/
void execute_pipe (command_t);

/*
A;B
Execute A, then B, give exit status of B
*/
void execute_sequence (command_t);

/*
A&&B

Execute A
	If true execute B
		If B is true, return true
	Return false
*/
void execute_and (command_t);

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
void execute_or (command_t);

/*
Execute redirects
Haven't figured this out completely yet
Should work with simple commands

But haven't checked subshell yet
*/
void execute_redirect(command_t);

/*
Executes simple, atomic commands
*/

void execute_simple (command_t);

/* 
Not implemented
*/
void execute_sequence (command_t);

/* 
Not implemented
*/
void execute_subshell (command_t);

void no_tt (command_t);
/*
Part 1c - not yet implemented
*/
void timetravel(command_t);

/*
This function deals with reading the files
*/
void filesetup(command_t);

/*
Our reac-command tokeniser stores words like this

"echo hello world"

What we need instead is something like

"echo" "hello" "world"

But we need this parsing only on the level of simple commands

That is the utility of this function

We are just using strok() for this function

Makes life easy

*/



