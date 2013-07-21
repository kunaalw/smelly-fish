/*
This is the timetravel function

We add things to a dependency pool and fork a separate process for a command if it doesn't share elements in the dependency pool with prior threads 

*/
void timetravel (command_stream_t);

/*

Helper Functions

*/

//Create pool of dependencies
void start_pool (char **, int, int );

//Add to pool of dependencies
void add_dependencies (char**, int, int);

//After a thread finishes running, remove its files from list of dependencies
void remove_dependencies (int);

//return 0 for false and 1 for true if a command has dependencies in pool
int checkdependencies (char**, int);

/* Data Structures */

/*
A file index which maintains the name of a file and an index of commands which use it AND have still not run

After a command is succesfully run, we remove the entry which marks that command's file as still in use

filename - character array
depcheck - integer array
fileindex * head; -> pointer to head
fileindex * next; -> pointer to next - this is the only one that seems to be necessary
fileindex * prev; -> pointer to prev
fileindex * tail; -> pointer to tail

And this is a lot of commenting. 

Is it worth it?

Time to navel gaze

Like so

Say command 2 (index 1) uses testfile

testfile - filename
0 - depcheck [0]
1 - [1]
1 - [2]
1 - [3]


After command 2 is executed, we have the following entry
testfile - filename
0 - depcheck [0]
0 - [1] - the change is here
1 - [2]
1 - [3]


*/
struct fileindex
{
char* filename;
int* depcheck;
struct fileindex * head;
struct fileindex * next;
struct fileindex * prev;
struct fileindex * tail;
};
//typedef struct fileindex file_index;
//typedef struct fileindex;
/*
Dependency Pool
A pool which maintains a log of the relationship between threads yet to run and files they use
*/
struct dep_pool
{
//file_index wordlist;
struct fileindex * wordlist;
int cmd_count;
int file_count;
};

typedef struct command_dependencies command_dependencies;
struct command_dependencies
  {
    char **list_of_files;
    int num_files;
  };


command_dependencies single_command_dependencies(command_t);
/*
typedef struct 
{
char **filelist;
int filecount;
}command_dependencies;

command_dependencies* single_command_dependencies(command_t);
*/
