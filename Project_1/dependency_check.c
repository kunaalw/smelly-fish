// UCLA CS 111 Lab 1 dependency-checking

#include "command.h"
#include "command-internals.h"
#include "execute-internals.h"
#include "alloc.h"
#include "timetravel.h"
#include <string.h>
#include <error.h>
#include <stdio.h>



command_dependencies single_command_dependencies(command_t c)
{
  // Files associated with command
  command_dependencies return_val;
  char** file_names = (char**) checked_malloc(0); 
  int num_files_associated = 0;
  
  if (c->type == AND_COMMAND || c->type == OR_COMMAND || c->type == SEQUENCE_COMMAND || c->type == PIPE_COMMAND)
    {
      command_dependencies left_val = single_command_dependencies(c->u.command[0]);
      command_dependencies right_val = single_command_dependencies(c->u.command[1]);
      return_val.num_files = left_val.num_files + right_val.num_files;
      return_val.list_of_files = (char**) checked_realloc(file_names, sizeof(char*)*return_val.num_files);
      
      int i = 0;
      for (i; i < left_val.num_files; i++)
	return_val.list_of_files[i] = left_val.list_of_files[i];

      int j = left_val.num_files;
      int k = 0; // number of repeats
      for (; j+k-(left_val.num_files) < right_val.num_files;)
	{
	  int l = 0;
	  int repeat = 0;
	  for (l; l < left_val.num_files; l++)
	    {
	      if (strcmp(right_val.list_of_files[j+k-(left_val.num_files)],left_val.list_of_files[l])) // repeat found
		{
		  k++;
		  repeat = 1;
		  break;
		}
	    }
	  if (repeat == 0)
	    {
	      return_val.list_of_files[j] = right_val.list_of_files[j+k-(left_val.num_files)];
	      j++;
	    }
	}
      return_val.num_files = j;
    }

  else if (c->type == SUBSHELL_COMMAND)
    {
      return_val = single_command_dependencies(c->u.subshell_command);
    }
  
  else if (c->type == SIMPLE_COMMAND)
    {
      
      command_dependencies return_val;
      
      char* single_word;   // single word from simple command
      int flags_over = 0;  // checks if looking at files/dependencies or files
      int in_quotes = 0;   // check if the given word is in quotes
      
      char* in_string = (char*) checked_malloc(sizeof(c->u.word));
      strcpy(in_string, c->u.word[0]);
      single_word = strtok (in_string, " ");

      if (single_word != NULL)
	{
	  single_word = strtok (NULL, " ");
	  while (single_word != NULL)
	    {
	      if (flags_over == 0)
		{
		  if (single_word[0] == '-');
		  else
		    {
		      flags_over = 1;
		      num_files_associated++;
		      
		      file_names = (char**) checked_realloc(file_names, sizeof(char*)*num_files_associated);
		      int length_of_word = strlen(single_word);

		      file_names[num_files_associated-1] = (char*) checked_malloc(length_of_word*sizeof(char));
		      strcpy(file_names[num_files_associated-1], single_word);
		    }
		}
	      else
		{
		  num_files_associated++;
		  
		  file_names = (char**) checked_realloc(file_names, sizeof(char*)*num_files_associated);
		  int length_of_word = strlen(single_word);
		  
		  file_names[num_files_associated-1] = (char*) checked_malloc(length_of_word*sizeof(char));
		  strcpy(file_names[num_files_associated-1], single_word);
		}
	      single_word = strtok (NULL, " ");
	    }
	}
      else error (1, 0, "ERROR: Empty simple command");

      if (c->input != NULL)
	{
	  // deleting extra whitespaces (preceding)
	  while (c->input[0] == ' ')
	    c->input = &(c->input[1]);

	  num_files_associated++;
	  file_names = (char**) checked_realloc(file_names, sizeof(char*)*num_files_associated);
	  file_names[num_files_associated-1] = (char*) checked_malloc(strlen(c->input)*sizeof(char));
	  strcpy(file_names[num_files_associated-1],c->input);
	}
      if (c->output != NULL)
	{
	  // deleting extra whitespaces (preceding)
	  while (c->output[0] == ' ')
	    c->output = &(c->output[1]);

	  num_files_associated++;
	  file_names = (char**) checked_realloc(file_names, sizeof(char*)*num_files_associated);
	  file_names[num_files_associated-1] = (char*) checked_malloc(strlen(c->output)*sizeof(char));
	  strcpy(file_names[num_files_associated-1],c->output);
	}
    }
  
  else
    error(1, 0, "ERROR: Bad type");
 
  return_val.list_of_files = file_names;
  return_val.num_files = num_files_associated;
  return return_val;
}
