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
#include <signal.h>
#include <sys/types.h>  
#include <stdlib.h>  
#include <error.h>
#include <errno.h>
#include <fcntl.h>			
#include <sys/stat.h>
#include <pthread.h>


// Creates the dependency table
parallel_data create_dependency_table (command_stream_t s)
{
  if (s == NULL) exit;
  if (s->num_commands == 0) exit;

  int size_of_file_array = 0;
  char** array_of_file_indices = (char**) checked_malloc(size_of_file_array*sizeof(char*));
  
  //int commands_status [s->num_commands]; // status 1 = runnable, status 2 = running, status 3 = ready, status 0 = completed successfully, status -1 = completed unsuccessful

  int* commands_status = (int*) calloc(s->num_commands, sizeof(int));

  int populate_status = 0;
  for (populate_status; populate_status < (s->num_commands); populate_status++)
    {
      commands_status[populate_status] = 1;
    }
  
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


// Function called when thread is spawned
void* run_thread (void* argument)
{
  command_t* foo = (command_t*) (argument);
  //printf("   -- SUB-STATUS: Command execution started\n");
  if (foo == NULL) error (1, 0, "ERROR: Thread given null pointer");
  else execute_command(*foo, 0);
  //printf("   -- SUB-STATUS: Command execution completed\n");
  return NULL;
}

// Returns status of thread - 1 = still running, 0 = exited/dead
int check_thread_status (pthread_t in_thread)
{
  if(pthread_kill(in_thread, 0) == 0)
    {
      return 1; 
    }
  return 0;
}

void timetravel(command_stream_t s)
{
  // The maximum number of threads that can exist at a given time
  int max_num_threads = 6;

  pthread_t threads[max_num_threads];
  int thread_cmd_running[max_num_threads];

  int a = 0;
  for (a; a < max_num_threads; a++)
    {
      thread_cmd_running[a] = -1;
    }

  if (s == NULL) return;
  if (s->num_commands == 0) return;

  parallel_data initialized = create_dependency_table(s);
  command_t* command_list = s->command_array;

  //printf(" -- STATUS: Dependency table created\n");
  //print_dependency_table(initialized);

  int exit_check = 0;
  int curr_num_threads = 0;
  
  while (exit_check == 0)
    {
      //printf(" -- STATUS: Enters the main loop\n");
      // Check to see if any threads have completed and update dependency table if they have
      int b = 0;
      for (b; b < max_num_threads; b++)
	{
	  // if this was actually a command, update dependency table and status
	  if (thread_cmd_running[b] != -1 && check_thread_status(threads[b]) == 0)
	    {
	      //printf(" -- STATUS: Thread with command %d is dead\n", thread_cmd_running[b]);
	      int cmd_completed = thread_cmd_running[b];
	      completed_nth_command (&initialized, cmd_completed, 0);
	      thread_cmd_running[b] = -1;
	      curr_num_threads--;
	    }
	}
      //curr_num_threads = b;

      //printf(" -- STATUS: Completed thread checking complete\n");
      // Check status
      int i = 0;
      exit_check = 1;
      for (i; i < initialized.num_cmds_rows; i++)
	{
	  if (initialized.status_table[i] != 0)
	    {
	      exit_check = 0;
	      break;
	    }
	}
      
      // If status is 1, exit
      if (exit_check == 1 && curr_num_threads == 0)
	{
	  return;
	}
      else if (exit_check == 1 && curr_num_threads > 0)
	{
	  error (1, 0, "ERROR: Implementation error - bad flag setting - threads still running");
	  return;
	}
      else if (exit_check == 1 && curr_num_threads < 0)
	{
	  error (1, 0, "ERROR: Implementation error - negative number of threads");
	  return;
	}  

      // If status is 0, do work
      else 
	{
	  // Check if anything has become ready
	  int m = 0;
	  for (m; m < initialized.num_cmds_rows; m++)
	    {
	      //printf("the status is %d\n", initialized.status_table[m]);
	      if (initialized.status_table[m] == 1) // runnable
		{
		  if (check_nth_command (&initialized, m) == 0)
		    {
		      initialized.status_table[m] = 3;
		      //printf(" -- STATUS: Command %d is now runnable\n", m);
		    }
		}
	    }

	  // Try to run anyone who is ready
	  int j = 0;
	  for (j; j < initialized.num_cmds_rows; j++)
	    {
	      // If a given command is ready and num_threads < max limit, run it!
	      if ((initialized.status_table[j] == 3) && (curr_num_threads < max_num_threads))
		{
		  //printf(" -- STATUS: Attempting to run command %d\n", j);
		  // create thread, update thread key table + num of threads, update command status
		  curr_num_threads++;
		  int k = 0;
		  for (k; k < max_num_threads; k++)
		    {
		      if (thread_cmd_running[k] == -1)
			{
			  //printf(" -- STATUS: Command %d is assigned thread %d\n", j, k);
			  thread_cmd_running[k] = j;
			  command_t* in_cmd = &(command_list[j]);
			  pthread_create(&(threads[k]), NULL, run_thread, in_cmd);
			  initialized.status_table[j] = 2;
			  break;
			}
		    }
		} 
	    }
	}
    }
    void *exit_status;
    int endlp = 0;
    for (endlp; endlp < max_num_threads; endlp++)
      pthread_join (threads[endlp], &exit_status);
}
