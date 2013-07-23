#include "command.h"
#include "command-internals.h"
#include "execute-internals.h"
#include "alloc.h"
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

 /*
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
printf("gets to timetravel loop\n");
tmp = single_command_dependencies (c->command_array[i]);
temp=pool->file_count;
pool->file_count=(temp+(tmp.num_files));
start_pool (tmp.list_of_files, tmp.num_files, cmd_count);
}

printf("%d", cmd_count);
//error(1, 0, "Timetravel not yet implemented");
}


//Start pool of dependencies
void start_pool (char ** files, int filecount, int cmd_count)
{
fui *head = (fui *)malloc(sizeof(fui)); //FUI - file use index
pool->wordlist = head;
int * numcommands = (int*)malloc(sizeof(int)*cmd_count);
head->depcheck=numcommands;
printf("gets here start_pool\n");
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
*/


void timetravel(command_stream_t s)
{ 
  if (s == NULL) return;
  if (s->num_commands == 0) return;

  int size_of_file_array = 0;
  char** array_of_file_indices = (char**) checked_malloc(size_of_file_array*sizeof(char*));
  
  int commands_status [s->num_commands]; // status 1 = live, status 0 = completed successfully, status -1 = unsuccessful
  int populate_status = 0;
  for (populate_status; populate_status < s->num_commands; populate_status++)
    commands_status[populate_status] = 1;
  
  // Find the number of files (to create a 2D array later) - and populate single file array
  int i = 0; 
  for(i; i < s->num_commands; i++)
    {
      //printf("Tries to find number of files\n");
      command_dependencies curr_dependency_retval =  single_command_dependencies(s->command_array[i]);

      int j = 0;
      for (j; j < curr_dependency_retval.num_files; j++)
	{
	  int already_added_flag = 0;

	  int k = 0;
	  for (k; k < size_of_file_array; k++)
	    {
	      if (strcmp(curr_dependency_retval.list_of_files[j], array_of_file_indices[k]) == 0)
		{
		  printf("The file '%s' is already present... and is saved as %s\n", curr_dependency_retval.list_of_files[j], array_of_file_indices[k]);
		  already_added_flag = 1;
		}
	    }

	  if (already_added_flag == 0)
	    {
	      //printf("So this should add %s!\n", curr_dependency_retval.list_of_files[j]);
	      size_of_file_array += 1;
	      array_of_file_indices = (char**) checked_realloc(array_of_file_indices, size_of_file_array*sizeof(char*));
	      array_of_file_indices[size_of_file_array-1] = curr_dependency_retval.list_of_files[j];
	    }
	}
    }

  // Make 2x2 array and populate it with all zeroes
  int dependency_table[s->num_commands][size_of_file_array];
  int row_init = 0;
  for (row_init; row_init < s->num_commands; row_init++)
    {
      int col_init = 0;
      for (col_init; col_init < size_of_file_array; col_init++)
	{
	  dependency_table[row_init][col_init] = 0;
	}
    }

  /*
   // Test - print table
  int q = 0;
  int t = 0;
  printf("\n\n*** DEPENDENCY TABLE ***\n");
  for (t; t < size_of_file_array; t++)
    {
      printf(" ~%s~ ", array_of_file_indices[t]);
    }
  printf("\n");
  for (q; q < (s->num_commands); q++)
    {
      int r = 0;
      for (r; r < size_of_file_array; r++)
	{
	  printf(" %d ", dependency_table[q][r]);
	}
      printf("\n");
    }
  */

  printf("Number of files is %d\n", size_of_file_array);

  // Fill in table properly
  int m = 0; 
  for(m; m < s->num_commands; m++)
    {
      command_dependencies curr_dependency_retval =  single_command_dependencies(s->command_array[m]);
      int n = 0;
      for (n; n < curr_dependency_retval.num_files; n++)
	{
	  int file_index = -1;
	  int p = 0;
	  for (p; p < size_of_file_array; p++)
	    {
	      if (strcmp(curr_dependency_retval.list_of_files[n], array_of_file_indices[p]) == 0)
		file_index = p;
	    }
	  if (file_index < 0) error (1, 0, "ERROR: Implementation error - bad dependency table (missing values)");
	  else 
	    {
	      dependency_table[m][file_index] = 1;
	    }
	}
    }
  
  // Test - print table
  int q = 0;
  int t = 0;
  printf("\n\n*** DEPENDENCY TABLE ***\n");
  for (t; t < size_of_file_array; t++)
    {
      printf(" ~%s~ ", array_of_file_indices[t]);
    }
  printf("\n");
  for (q; q < (s->num_commands); q++)
    {
      int r = 0;
      for (r; r < size_of_file_array; r++)
	{
	  printf(" %d ", dependency_table[q][r]);
	}
      printf("\n");
    }
  
}

/*
typedef struct command_dependencies command_dependencies;
struct command_dependencies
  {
    char **list_of_files;
    int num_files;
  };
 */
