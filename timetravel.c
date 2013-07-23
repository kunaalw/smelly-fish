#include "command.h"
#include "command-internals.h"
#include "execute-internals.h"
#include "alloc.h"
#include "timetravel.h"
#include <error.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>    /* POSIX Threads */
#include <sys/wait.h> 
#include <unistd.h>  
#include <sys/types.h>  
#include <stdlib.h>  
#include <error.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>			
#include <sys/stat.h>
#define MAXTHREADS 5

pthread_t tid[MAXTHREADS];

 
 //global variable for dependency pool
parallel_data global_table;

 //global variable for commands
//command_stream_t *global_command = (command_stream_t*) checked_malloc(sizeof(command_stream_t*));
command_stream_t global_command;

// Creates the dependency table
parallel_data create_dependency_table (command_stream_t s)
{
  if (s == NULL) exit;
  if (s->num_commands == 0) exit;

  int size_of_file_array = 0;
  char** array_of_file_indices = (char**) checked_malloc(size_of_file_array*sizeof(char*));
  
  int commands_status [s->num_commands]; // status 1 = runnable, status 2 = running,  status 0 = completed successfully, status -1 = unsuccessful
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
		  //printf("The file '%s' is already present... and is saved as %s\n", curr_dependency_retval.list_of_files[j], array_of_file_indices[k]);
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
  int* values = (int*) calloc((s->num_commands)*(size_of_file_array), sizeof(int));
  int** dependency_table = (int**) checked_malloc(s->num_commands*sizeof(int*));
  int setit = 0;
  for (setit; setit < s->num_commands; setit++)
    {
      dependency_table[setit] = values + (setit*size_of_file_array);
    }

  //int dependency_table[s->num_commands][size_of_file_array];
  int row_init = 0;
  for (row_init; row_init < s->num_commands; row_init++)
    {
      int col_init = 0;
      for (col_init; col_init < size_of_file_array; col_init++)
	{
	  dependency_table[row_init][col_init] = 0;
	}
    }

  // printf("Number of files is %d\n", size_of_file_array);

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

  parallel_data ret_data;
  ret_data.dependency_table = dependency_table;
  ret_data.file_reference_key = array_of_file_indices;
  ret_data.status_table = commands_status;
  ret_data.num_files_cols = size_of_file_array;
  ret_data.num_cmds_rows = s->num_commands;

  return ret_data;
}

// Prints dependency table (for debugging)
void print_dependency_table (parallel_data initialized)
{
  int q = 0;
  int t = 0;
  printf("\n\n*** DEPENDENCY TABLE ***\n");
  for (t; t < initialized.num_files_cols; t++)
    {
      printf(" ~%s~ ", initialized.file_reference_key[t]);
    }
  printf("\n");
  for (q; q < (initialized.num_cmds_rows); q++)
    {
      int r = 0;
      for (r; r < initialized.num_files_cols; r++)
	{
	  printf(" %d ", initialized.dependency_table[q][r]);
	}
      printf("\n");
    }
}

// Call when the command with index n is completed (nth command from input)
void completed_nth_command (parallel_data* in_data, int cmd_completed, int complete_status)
{
  if (cmd_completed > in_data->num_cmds_rows || cmd_completed < 0)  error (1, 0, "ERROR: bad update value given to dependency table");
 
  int i = 0;
  for (i; i < in_data->num_files_cols; i++)
    {
      in_data->dependency_table[cmd_completed][i] = 0;
    }
  
  in_data->status_table[cmd_completed] = complete_status;
}

// Call to check if nth command can be run (i.e. has no pending dependency - or no command needs to be run before it)
// return value = 0 means can be run; return value = -1 means cannot be run
int check_nth_command (parallel_data* in_data, int cmd_to_check)
{
  if (cmd_to_check > in_data->num_cmds_rows || cmd_to_check < 0)  error (1, 0, "ERROR: bad command number given to dependency table to check");
  int ret_val = 0;

  int i = 0;
  for (i; i < in_data->num_files_cols; i++)
    {
      if ((in_data->dependency_table)[cmd_to_check][i] == 1)
	{
	  int j = 0;
	  for (j; j < cmd_to_check; j++)
	    {
	      if ((in_data->dependency_table)[j][i] == 1)
		{
		  ret_val = -1;
		  return ret_val;
		}
	    }
	}
    }

  return ret_val;
}

void timetravel(command_stream_t s)
{ 
 int *retvals[MAXTHREADS];
  if (s == NULL) return;
  if (s->num_commands == 0) return;
	int err;
  global_table = create_dependency_table(s);
  //global_command = &s;
  global_command = s;
  int fill=0;
  for(fill=0;fill<s->num_commands;fill++)
  {
  global_table.status_table[fill]=1;//Set all commands to waiting
  }

  int finished=0;
while(finished==0)
{
int threadindex=0;
for(threadindex=0;threadindex<MAXTHREADS;threadindex++)
{	
        err = pthread_create(&(tid[threadindex]), NULL, &parallelexecute, NULL);
        if (err != 0)
            printf("\ncan't create thread :[%s]", strerror(err));
        else
            printf("\n Thread created successfully\n");
}

for(threadindex=0;threadindex<MAXTHREADS;threadindex++)
{	
 pthread_join(tid[threadindex], (void**)&(retvals[threadindex]));
}

 
/*
struct parallel_data
  {
    int** dependency_table;  // the main dependency table
    char** file_reference_key;  // find index by name (use strcmp in a loop)
    int* status_table;  // you know you are done when all the statuses are 0 (none should ever be -1)
    int num_files_cols;  // number of columns/files
    int num_cmds_rows;  // number of rows/commands 
  };
*/
if(completecheck(global_table)==0)
{
finished=1;
}


}
 // print_dependency_table(global_table);
 //print_command(global_command->command_array[1]);
}

int completecheck(parallel_data data)
{
// status 1 = runnable, status 2 = running,  status 0 = completed successfully, status -1 = unsuccessful
int i=0;
for(i=0;i<data.num_cmds_rows;i++)
{
if(data.status_table[i]!=0)
return -1;
}
return 0;
}


void* parallelexecute(void *arg)
{
int i;
for(i=0;i<global_table.num_cmds_rows;i++)
{
// status 1 = runnable, status 2 = running,  status 0 = completed successfully, status -1 = unsuccessful
if(global_table.status_table[i]==1&&(check_nth_command(&global_table, i))==0)
{
global_table.status_table[i]=2;
execute_command(global_command->command_array[i], 0);
completed_nth_command(&global_table, i, 0);
break;
}
}
//We should never get here
completed_nth_command(&global_table, i, -1);
assert( !"Unreachable code hit" );
}
