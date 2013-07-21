#include "command.h"
#include "command-internals.h"
#include "execute-internals.h"
#include "timetravel.h"
#include <error.h>
#include <stdio.h>
#include <string.h>

#include <sys/wait.h> 
#include <unistd.h>  
#include <sys/types.h>  
#include <stdlib.h>  
#include <error.h>
#include <errno.h>

#include <fcntl.h>			
#include <sys/stat.h>

typedef struct dep_pool dp;
typedef struct fileindex fui;	
typedef struct command_dependencies cmd_dep;
dp *pool; //dependency pool
/*
command_dependencies* single_command_dependencies (command_t c)
{
command_dependencies *cd;
cd = (command_dependencies *)malloc(sizeof(command_dependencies));
cd->filecount=5;
int i=0;
for(i=0;i<5;i++)
{
cd->filelist[i]=(char*)malloc(50 * sizeof(char));
strcpy(cd->filelist[i], "hello");
}
for(i=0;i<5;i++)
{
printf("%s", cd->filelist[i]);
}


return cd;
}
*/
void timetravel(command_stream_t c)
{
	int cmd_count; //command count
	cmd_count=c->num_commands;
	pool = (dp *)malloc(sizeof(dp)); //FUI - file use index
	pool->cmd_count=cmd_count;
	pool->file_count=0;
	int i=0;
	int temp;
command_dependencies tmp;
		for(i=0;i<cmd_count;i++)
{
tmp = single_command_dependencies (c->command_array[i]);
temp=pool->file_count;
pool->file_count=(temp+(tmp.num_files));
start_pool (tmp.list_of_files, tmp.num_files, cmd_count);
}
//printf("%d", cmd_count);
//error(1, 0, "Timetravel not yet implemented");
}


//Start pool of dependencies
void start_pool (char ** files, int filecount, int cmd_count)
{
fui *head = (fui *)malloc(sizeof(fui)); //FUI - file use index
pool->wordlist = head;
int * numcommands = (int*)malloc(sizeof(int)*cmd_count);
head->depcheck=numcommands;

//return;
}

//Add to pool of dependencies
void add_dependencies (char** word, int index, int numfiles)
{
//return;
}

//After a thread finishes running, remove its files from list of dependencies
void remove_dependencies (int count)
{
//return;
}

//return 0 for false and 1 for true if a command has dependencies in pool
int checkdependencies (char** word, int index)
{
return 0;
}
