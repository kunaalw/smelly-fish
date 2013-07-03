// UCLA CS 111 Lab 1 command reading


// Project header files
#include "command.h"
#include "command-internals.h"
#include "alloc.h"


// C Libraries
#include <error.h>
#include <stdio.h>
#include <ctype.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>


// Type of token
typedef enum
  {
    AND_TOKEN,            // &&
    SEQUENCE_TOKEN,       // ;
    OR_TOKEN,             // ||
    PIPE_TOKEN,           // |
    SUBSHELL_OPEN_TOKEN,  // (
    SUBSHELL_CLOSE_TOKEN, // )
    REDIRECT_LEFT_TOKEN,  // <
    REDIRECT_RIGHT_TOKEN, // >
    NEWLINE_TOKEN,        // \n
    SIMPLE_TOKEN,         // word
    EMPTY_TOKEN,          // used for buffer
  } token_type;


// A token (type + value) + preceding token in doubly linked list
typedef struct token token;
struct token
  {
    token_type type;
    union
      {
        // for ALL NON-WORD TOKENS
        int no_data;

        // for WORD TOKENS
        struct
          {
            int word_length;
            char* word_content;
	  } simple_token;

      } curr;

    token* prev_token;
    token* next_token;
  };


void throw_error (char* error_message)
{
  printf("%s", error_message);
  printf("\n");
  exit(EXIT_FAILURE);
}


void token_type_printer(token* current_token) 
{
  switch(current_token->type) 
    {
    case AND_TOKEN : printf("AND_TOKEN"); break;
    case OR_TOKEN : printf("OR_TOKEN"); break;
    case PIPE_TOKEN : printf("PIPE_TOKEN"); break;
    case SUBSHELL_OPEN_TOKEN : printf("SUBSHELL_OPEN_TOKEN"); break;
    case SUBSHELL_CLOSE_TOKEN : printf("SUBSHELL_CLOSE_TOKEN"); break;
    case REDIRECT_LEFT_TOKEN : printf("REDIRECT_LEFT_TOKEN"); break;
    case REDIRECT_RIGHT_TOKEN : printf("REDIRECT_RIGHT_TOKEN"); break; 
    case NEWLINE_TOKEN : printf("NEWLINE_TOKEN"); break;
    case SIMPLE_TOKEN : printf("SIMPLE_TOKEN   -----   "); printf("%s", current_token->curr.simple_token.word_content); break;
    case EMPTY_TOKEN : printf("EMPTY_TOKEN"); break;
    case SEQUENCE_TOKEN : printf("SEQUENCE_TOKEN"); break;
    }
  printf("\n");
}




token *make_single_token (token_type input_token_type, token* input_prev_token) 
{
  // ## MEMORY ALLOCATION (I) ##
  // ## Deallocate after command stream has been created ##
  // ## Deallocation should occur before end of make_command_stream ##
  token* return_token = (token*) checked_malloc(sizeof(token));

  return_token->type = input_token_type;
  return_token->curr.no_data = 0;

  return_token->prev_token = input_prev_token;
  return_token->next_token = NULL;
  if (input_prev_token != NULL)
    {
      // printf("The previous token type is: ");
      // token_type_printer(input_prev_token);
      input_prev_token->next_token = return_token;
    }
  return return_token;
}


token *make_simple_token (token* input_prev_token, char* input_token_content, int input_token_length) 
{
  // ## MEMORY ALLOCATION (I) ##
  // ## Deallocate after command stream has been created ##
  // ## Deallocation should occur before end of make_command_stream ##
  token* return_token = (token*) checked_malloc(sizeof(token));
  return_token->type = SIMPLE_TOKEN;
  
  // Trim trailing space
  char* end = input_token_content + strlen(input_token_content) - 1;
  while(end > input_token_content && isspace(*end)) 
    {
      end--;
      input_token_length--;
    }
  
  // Write new null terminator
  *(end+1) = 0;

  return_token->curr.simple_token.word_length = input_token_length;
  return_token->curr.simple_token.word_content = (char*) checked_malloc(sizeof(char)*input_token_length);
  //*return_token->curr.simple_token.word_content = *input_token_content;
  strcpy(return_token->curr.simple_token.word_content, input_token_content);

  // printf("Input token content is: %s\n", input_token_content);
  // printf("Input token length is: %d\n", input_token_length);
  // printf("Saved token content is: %s\n", return_token->curr.simple_token.word_content);

  return_token->prev_token = input_prev_token;
  return_token->next_token = NULL;
  if (input_prev_token != NULL)
    {
      // printf("The previous token type is: ");
      // token_type_printer(input_prev_token);
      input_prev_token->next_token = return_token;
    }
  return return_token;
}


// Uses the buffer to make a token
// Returns 1 if an AND_TOKEN is created i.e. if buffer had '&' and current char was '&'
// Returns 2 if an OR_TOKEN is created i.e. if buffer had '|' and current char was '|'
// Returns 0 if any other token created, -1 if called on an empty buffer
int buffer_tokenize 
(char curr_char, char* buffer, token_type* buffer_type, int *buffer_size, token **curr_token, token **curr_minus_one_token, int* is_first, token **first)
{
  int curr_usage = -1;
  if (buffer_size != 0)
    {
      curr_usage = 0;
      if (*buffer_type == AND_TOKEN) 
	{
	  if (curr_char == '&')
	    {
	      *curr_minus_one_token = *curr_token;
	      *curr_token = make_single_token (AND_TOKEN, *curr_minus_one_token);
	      curr_usage = 1;
	    }
	  else
	    {
	      throw_error("ERROR: Single '&' symbol found.");
	    }
	}
      else if (*buffer_type == PIPE_TOKEN)
	{
	  if (curr_char == '|')
	    {
	      *curr_minus_one_token = *curr_token;
	      *curr_token = make_single_token (OR_TOKEN, *curr_minus_one_token);
	      curr_usage = 2;
	    }
	  else
	    {
	      *curr_minus_one_token = *curr_token;
	      *curr_token = make_single_token (PIPE_TOKEN, *curr_minus_one_token);
	    }
	}
      else if (*buffer_type == SIMPLE_TOKEN)
	{
	  *curr_minus_one_token = *curr_token;
	  *curr_token = make_simple_token (*curr_minus_one_token, buffer, *buffer_size);
	}

      memset(buffer, '\0', sizeof(char)*(*buffer_size));
      buffer = (char*) checked_realloc(buffer, 0);
      *buffer_size = 0;
      *buffer_type = EMPTY_TOKEN;
    }

  if ((*is_first == 0) && (*curr_token != NULL))
    {
      *first = *curr_token;
      *is_first = 1;
    } 
  return curr_usage;
}


// Parses input buffer into a token stream. Returns first token in doubly linked lsit
token *make_token_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  char in_char;
  token *first_token = NULL;
  token *curr_minus_one_token = NULL;
  token *curr_token = NULL;

  // Parenthesis mismatch checker
  int delta_left_right_paren = 0;

  // Buffer (to hold possibly incomplete tokens)
  char* buffer = (char*) checked_malloc(0);
  memset(buffer, '\0', 0);
  token_type buffer_type = EMPTY_TOKEN;
  int buffer_size = 0;
  int is_first_token = 0;

  regex_t check_word;
  regcomp(&check_word, "[A-Za-z0-9!%+,-./:@^_]", REG_EXTENDED);

  while ((in_char = get_next_byte(get_next_byte_argument)) != EOF)
    {
      printf("%c",in_char);
      if (in_char == '#')
	{
	  while ((in_char = get_next_byte(get_next_byte_argument)) != '\n');
	}
      if (in_char == ';')
	{
	  buffer_tokenize (in_char, buffer, &buffer_type, &buffer_size, &curr_token, &curr_minus_one_token, &is_first_token, &first_token);
	  curr_minus_one_token = curr_token;
	  curr_token = make_single_token (SEQUENCE_TOKEN, curr_minus_one_token);
	}

      else if (in_char == '|')
	{
	  int buff_tokenize_result =  buffer_tokenize (in_char, buffer, &buffer_type, &buffer_size, &curr_token, &curr_minus_one_token, &is_first_token, &first_token);
	  if (buff_tokenize_result != 2) 
	    {
	      if (buffer_type == EMPTY_TOKEN) 
		{
		  buffer_size++;
		  buffer = (char*) checked_realloc(buffer, sizeof(char)*buffer_size);
		  buffer[buffer_size-1] = in_char;
		  buffer_type = PIPE_TOKEN;
		}
	    }
	}

      else if (in_char == '&')
	{
	  int buff_tokenize_result =  buffer_tokenize (in_char, buffer, &buffer_type, &buffer_size, &curr_token, &curr_minus_one_token, &is_first_token, &first_token);
	  if (buff_tokenize_result != 1)
	    {
	      buffer_size++;
	      buffer = (char*) checked_realloc(buffer, sizeof(char)*buffer_size);
	      buffer[buffer_size-1] = in_char;
	      buffer_type = AND_TOKEN;
	    }
	}

      else if (in_char == '(')
	{
	  buffer_tokenize (in_char, buffer, &buffer_type, &buffer_size, &curr_token, &curr_minus_one_token, &is_first_token, &first_token);
	  curr_minus_one_token = curr_token;
	  curr_token = make_single_token (SUBSHELL_OPEN_TOKEN, curr_minus_one_token);
	  delta_left_right_paren++;
	}

      else if (in_char == ')') 
	{
	  buffer_tokenize (in_char, buffer, &buffer_type, &buffer_size, &curr_token, &curr_minus_one_token, &is_first_token, &first_token);
	  curr_minus_one_token = curr_token;
	  curr_token = make_single_token (SUBSHELL_CLOSE_TOKEN, curr_minus_one_token);
	  delta_left_right_paren--;
	}

      else if (in_char == '<')
	{
	  buffer_tokenize (in_char, buffer, &buffer_type, &buffer_size, &curr_token, &curr_minus_one_token, &is_first_token, &first_token);
	  curr_minus_one_token = curr_token;
	  curr_token = make_single_token (REDIRECT_LEFT_TOKEN, curr_minus_one_token);
	}

      else if (in_char == '>')
	{
	  buffer_tokenize (in_char, buffer, &buffer_type, &buffer_size, &curr_token, &curr_minus_one_token, &is_first_token, &first_token);
	  curr_minus_one_token = curr_token;
	  curr_token = make_single_token (REDIRECT_RIGHT_TOKEN, curr_minus_one_token);
	}

      else if (in_char == '\n')
	{
	  buffer_tokenize (in_char, buffer, &buffer_type, &buffer_size, &curr_token, &curr_minus_one_token, &is_first_token, &first_token);
	  curr_minus_one_token = curr_token;
	  curr_token = make_single_token (NEWLINE_TOKEN, curr_minus_one_token);
	}

      else if (regexec(&check_word, &in_char, (size_t) 0, NULL, 0) == 0) // SIMPLE_TOKEN
	{
	  if (buffer_type != EMPTY_TOKEN && buffer_type != SIMPLE_TOKEN) 
	    {
	      buffer_tokenize (in_char, buffer, &buffer_type, &buffer_size, &curr_token, &curr_minus_one_token, &is_first_token, &first_token);
	    }
	  
	  buffer_size++;
	  buffer = (char*) checked_realloc(buffer, sizeof(char)*buffer_size);
	  buffer[buffer_size-1] = in_char;
	  buffer_type = SIMPLE_TOKEN;
	}

      else if (in_char == ' ' || in_char == '\t')
	{
	  if(buffer_type == SIMPLE_TOKEN)
	    {
	      buffer_size++;
	      buffer = (char*) checked_realloc(buffer, sizeof(char)*buffer_size);
	      buffer[buffer_size-1] = in_char;
	      buffer_type = SIMPLE_TOKEN;
	    }
	  else buffer_tokenize (in_char, buffer, &buffer_type, &buffer_size, &curr_token, &curr_minus_one_token, &is_first_token, &first_token);
	}

      else // BAD_TOKEN
	{
	  // ### 100 is a magic number here - not a problem since expected output, but still try to fix.
	  char error_message[100];
	  sprintf(error_message,  "ERROR: Malformed expression - the character '%c' is illegal", in_char);
	  throw_error(error_message);
	}

      // Set the head of the list
      if ((is_first_token == 0) && (curr_token != NULL))
	{
	  first_token = curr_token;
	  is_first_token = 1;
	}
    }

  buffer_tokenize('~', buffer, &buffer_type, &buffer_size, &curr_token, &curr_minus_one_token, &is_first_token, &first_token);
  
  if (delta_left_right_paren != 0)
    {
      char* error_message = "ERROR: Malformed expression - parenthesis mismatch";
      throw_error(error_message);
    }
  
  free(buffer);
  if (first_token == NULL) printf("WARNING: No tokens could be generated from the given input\n");
  return first_token;
}

void clean_token_stream (token* input_token_stream)
{
  // Newline clean-up
  token* stored_token_stream = input_token_stream;
  while (input_token_stream != NULL)
    {
      if (input_token_stream->type == NEWLINE_TOKEN)
	{
	  // If begins with a newline, delete newline
	  if (input_token_stream->prev_token == NULL)
	    {
	      token* delete_token = input_token_stream;
	      input_token_stream = input_token_stream->next_token;
	      stored_token_stream = input_token_stream;
	      printf("\n\n\nGETS HERE\n\n\n");

	      if (input_token_stream->next_token != NULL)
		(input_token_stream->next_token)->prev_token = NULL;
	     
	      free(delete_token);
	    }
	  // If two newlines, delete one
	  if ((input_token_stream->next_token != NULL) && ((input_token_stream->next_token)->type == NEWLINE_TOKEN))
	    {
	      token* delete_token = input_token_stream;
	      if (input_token_stream->prev_token != NULL)
		(input_token_stream->prev_token)->next_token = input_token_stream->next_token;

	      if (input_token_stream->next_token != NULL)
		(input_token_stream->next_token)->prev_token = input_token_stream->prev_token;
	     
	      input_token_stream = input_token_stream->next_token;
	      free(delete_token);
	    }
	  // If succeeded by anything other than (, ) or a word - BOOM!! ERROR!!
	  else if ((input_token_stream->next_token != NULL) &&
		   ((input_token_stream->next_token)->type != SIMPLE_TOKEN) && 
		   ((input_token_stream->next_token)->type != SUBSHELL_OPEN_TOKEN) && 
		   ((input_token_stream->next_token)->type != SUBSHELL_CLOSE_TOKEN))
	    {
	      char* error_message = "ERROR: Malformed expression - unexpected preceding newline character";
	      //throw_error(error_message);
	    }
	  // If preceeded by <, > - BOOM!! ERROR!!
	  else if ((input_token_stream->prev_token != NULL) && 
		   (((input_token_stream->prev_token)->type == REDIRECT_LEFT_TOKEN) || 
		    ((input_token_stream->prev_token)->type == REDIRECT_RIGHT_TOKEN)))
	    {
	      char* error_message = "ERROR: Malformed expression - unexpected succeeding newline character";
	      //throw_error(error_message);
	    }
	  // If preceeeded by anything other than a simple word, delete the newline!
	  else if ((input_token_stream->prev_token != NULL) && 
		   ((input_token_stream->prev_token)->type != SIMPLE_TOKEN) && 
		   ((input_token_stream->prev_token)->type != SUBSHELL_CLOSE_TOKEN))
	    {
	      token* delete_token = input_token_stream;
	      if (input_token_stream->prev_token != NULL)
		(input_token_stream->prev_token)->next_token = input_token_stream->next_token;

	      if (input_token_stream->next_token != NULL)
		(input_token_stream->next_token)->prev_token = input_token_stream->prev_token;
	     
	      input_token_stream = input_token_stream->next_token;
	      free(delete_token);
	      }
	  // If it's none of these, it clearly signifies a new command - so change to sequence type
	  else
	    {
	      input_token_stream->type = SEQUENCE_TOKEN;
	      input_token_stream = input_token_stream->next_token;
	    }
	}
      else
	{
	  input_token_stream = input_token_stream->next_token;
	}
    }
  
  input_token_stream = stored_token_stream;
  // Sequence (semicolon) clean-up
  while (input_token_stream != NULL)
    {
      if (input_token_stream->type == SEQUENCE_TOKEN)
	{
	  // If begins with a semicolon, delete semicolon
	  if (input_token_stream->prev_token == NULL)
	    {
	       token* delete_token = input_token_stream;

	      if (input_token_stream->next_token != NULL)
		(input_token_stream->next_token)->prev_token = NULL;
	     
	      input_token_stream = input_token_stream->next_token;
	      free(delete_token);
	      }
	  // If two semicolons, delete one
	  if ((input_token_stream->next_token != NULL) && ((input_token_stream->next_token)->type == SEQUENCE_TOKEN))
	    {
	      token* delete_token = input_token_stream;
	      if (input_token_stream->prev_token != NULL)
		(input_token_stream->prev_token)->next_token = input_token_stream->next_token;

	      if (input_token_stream->next_token != NULL)
		(input_token_stream->next_token)->prev_token = input_token_stream->prev_token;
	     
	      input_token_stream = input_token_stream->next_token;
	      free(delete_token);
	    }
	  // If succeeded by anything other than (, ) or a word - BOOM!! ERROR!!
	  else if ((input_token_stream->next_token != NULL) &&
		   ((input_token_stream->next_token)->type != SIMPLE_TOKEN) && ((input_token_stream->next_token)->type != SUBSHELL_OPEN_TOKEN) && ((input_token_stream->next_token)->type != SUBSHELL_CLOSE_TOKEN))
	    {
	      char* error_message = "ERROR: Malformed expression - unexpected preceding sequence character";
	      throw_error(error_message);
	    }
	  // If preceeded by <, > - BOOM!! ERROR!!
	  else if ((input_token_stream->prev_token != NULL) && (((input_token_stream->prev_token)->type == REDIRECT_LEFT_TOKEN) || ((input_token_stream->prev_token)->type == REDIRECT_RIGHT_TOKEN)))
	    {
	      char* error_message = "ERROR: Malformed expression - unexpected succeeding sequence character";
	      throw_error(error_message);
	    }
	  // If it's none of these, continue
	  else
	    {
	      input_token_stream = input_token_stream->next_token;
	    }
	}
      else
	{
	  input_token_stream = input_token_stream->next_token;
	}
    }

  input_token_stream = stored_token_stream;
  /*// Malformed clean-up
  while (input_token_stream != NULL)
    {
      printf("Stuck here\n");
      if ((input_token_stream->type == REDIRECT_LEFT_TOKEN) || (input_token_stream->type == REDIRECT_RIGHT_TOKEN))
	{
	  if ((input_token_stream->next_token == NULL) || input_token_stream->prev_token == NULL)
	    throw_error("ERROR: Bad redirection - missing word");
	  else if (((input_token_stream->next_token)->type != SIMPLE_TOKEN) || ((input_token_stream->prev_token)->type != SIMPLE_TOKEN))
	    throw_error("ERROR: Unexpected token before or after redirection");
	}
      input_token_stream = input_token_stream->next_token;
    }
  */
}


token** complete_command_divider (token* input_token_stream, int* num_commands)
{
  token* stored_token_stream = input_token_stream;
  // Count number of complete commands
  *num_commands = 0;
  while (input_token_stream != NULL)
    {
      if (input_token_stream->type == SEQUENCE_TOKEN) (*num_commands)++;
      input_token_stream = input_token_stream->next_token;
    }
  
  input_token_stream = stored_token_stream;
  // Create array of token pointers (head token pointers); one entry per complete command
  // ## MEMORY ALLOCATION (III) ##
  // ## Deallocate after command stream has been created ##
  // ## Deallocation should occur before end of make_command_stream ##
  token** tokenized_command_array = (token**) checked_malloc(sizeof(token*)*(*num_commands));
  if (*num_commands > 0)
    {
      int i = 0;
      tokenized_command_array[i] = input_token_stream;
      i++;
      input_token_stream = input_token_stream->next_token;
      while (input_token_stream != NULL)
	{
	  if (input_token_stream->type == SEQUENCE_TOKEN)
	    {
	      token* delete_token = input_token_stream;
	      if (input_token_stream->prev_token != NULL)
		(input_token_stream->prev_token)->next_token = NULL;

	      if (input_token_stream->next_token != NULL)
		{
		  (input_token_stream->next_token)->prev_token = NULL;
		  tokenized_command_array[i] = input_token_stream->next_token;
		  i++;
		}
	     
	      input_token_stream = input_token_stream->next_token;
	      free(delete_token);
	    }
	  else
	    {
	      input_token_stream = input_token_stream->next_token;
	    }
	}
    }
  return tokenized_command_array;
}

// @Avinash: Follow this style for generating commands from tokens -Kunaal
command_t make_single_command (token* tokenized_command)
{
  printf("Make new command\n");
  token* first_token = tokenized_command;
  token* first_non_subshell_token = tokenized_command;
  token* current_token = tokenized_command;
  command_t return_command = NULL;

  // Subshell command?
  if(current_token->type == SUBSHELL_OPEN_TOKEN)
    {
      token* last_token = current_token;
      while (last_token->next_token != NULL)
	last_token = last_token->next_token;
      printf("The last token is of type: ");
      token_type_printer(last_token);
      if(last_token->type == SUBSHELL_CLOSE_TOKEN)
	{
	  token* last_in_subshell = last_token->prev_token;
	  token* first_in_subshell = first_token->next_token;
	  first_in_subshell->prev_token = NULL;
	  last_in_subshell->next_token = NULL;
	  
	  return_command = (command_t) checked_malloc(sizeof(struct command));
	  
	  command_t ret_subshell_command = (command_t) checked_malloc(sizeof(struct command));
	  ret_subshell_command = make_single_command(first_in_subshell);
	  
	  return_command->type = SUBSHELL_COMMAND;
	  return_command->u.subshell_command = ret_subshell_command;
	  return_command->status = -1;
	  return_command->input = NULL;
	  return_command->output = NULL;
	  printf("Goes in the subshell if statement\n");
	  return return_command;
	}

      else 
	{
	  current_token = first_token;
	  int counter = 0; 		
	  token* subshell_first = current_token->next_token;
	  while(current_token != NULL)
	    {
	      if(current_token->type == SUBSHELL_OPEN_TOKEN)
		counter++;
	      if(current_token->type == SUBSHELL_CLOSE_TOKEN)
		counter--;
	      token_type_printer(current_token);
	      printf("Right now the counter is %d\n\n", counter);
	      if(counter == 0)
		{
		  printf("It should break here\n");
		  break;
		}
	      current_token=current_token->next_token;
	    }
	  first_non_subshell_token = current_token->prev_token;
	}
    }


  // Try to find an AND or an OR:
  current_token = first_non_subshell_token;
  while(current_token != NULL)
    {
      if((current_token->type == OR_TOKEN) || (current_token->type == AND_TOKEN))
	{
	  return_command = (command_t) checked_malloc(sizeof(struct command));
	  if (current_token->type == OR_TOKEN) return_command->type = OR_COMMAND;
	  if (current_token->type == AND_TOKEN) return_command->type = AND_COMMAND;
	  token* left_branch;
	  token* right_branch;

	  // Taking care of the left branch
	  if (current_token->prev_token == NULL) throw_error("ERROR: Malformed expression - boolean without preceding expression");
	  else
	    {
	      (current_token->prev_token)->next_token = NULL;
	      left_branch = first_token;
	    }

	  // Taking care of the right branch
	  if (current_token->next_token == NULL) throw_error("ERROR: Malformed expression - boolean without succeeding expression");
	  else
	    {
	      right_branch = current_token->next_token;
	      right_branch->prev_token = NULL;
	    }

	  return_command->u.command[0] = make_single_command(left_branch);
	  return_command->u.command[1] = make_single_command(right_branch);
	  return_command->status = -1;
	  return_command->input = NULL;
	  return_command->output = NULL;

	  return return_command;
	}
      else current_token = current_token->next_token;
    }

  // Couldn't find an AND/OR... bleh! Maybe a PIPE instead?
  current_token = first_non_subshell_token;
  while(current_token != NULL)
    {
      if(current_token->type == PIPE_TOKEN)
	{
	  return_command = (command_t) checked_malloc(sizeof(struct command));
	  return_command->type = PIPE_COMMAND;
	  token* left_branch;
	  token* right_branch;

	  // Taking care of the left branch
	  if (current_token->prev_token == NULL) throw_error("ERROR: Malformed expression - pipe without preceding expression");
	  else
	    {
	      (current_token->prev_token)->next_token = NULL;
	      left_branch = first_token;
	    }

	  // Taking care of the right branch
	  if (current_token->next_token == NULL) throw_error("ERROR: Malformed expression - pipe without succeeding expression");
	  else
	    {
	      right_branch = current_token->next_token;
	      right_branch->prev_token = NULL;
	    }

	  return_command->u.command[0] = make_single_command(left_branch);
	  return_command->u.command[1] = make_single_command(right_branch);
	  return_command->status = -1;
	  return_command->input = NULL;
	  return_command->output = NULL;
	  return return_command;
	}
      else current_token = current_token->next_token;
    }

  // Try to find a word aka SIMPLE_TOKEN!
  current_token = first_non_subshell_token;
  while(current_token != NULL)
    {
      if(current_token->type == SIMPLE_TOKEN)
	{
	  return_command = (command_t) checked_malloc(sizeof(struct command));
	  return_command->type = SIMPLE_COMMAND;
	  
	  return_command->u.word = (char**) checked_malloc(sizeof(char*));
	  return_command->u.word[0] = (current_token->curr).simple_token.word_content;

	  return_command->status = -1;
	  return_command->input = NULL;
	  return_command->output = NULL;
	  int redirect_flag = 0;
	  
	  if (current_token->next_token != NULL)
	    {
	      if ((current_token->next_token)->type == REDIRECT_LEFT_TOKEN)
		{
		  if ((current_token->next_token)->next_token == NULL)
		    throw_error("ERROR: Left redirection empty"); 
		  
		  if (((current_token->next_token)->next_token)->type != SIMPLE_TOKEN)
		    throw_error("ERROR: Left redirection not followed by a simple token");
		  
		  current_token = (current_token->next_token)->next_token;

		  return_command->input = (char*) checked_malloc(sizeof(char)*(current_token->curr.simple_token.word_length));

		  return_command->input = strncpy((return_command->input),(current_token->curr.simple_token.word_content),(current_token->curr.simple_token.word_length));
		  redirect_flag = 1;
		}
	      if (current_token->next_token != NULL)
		{
		  if (((current_token->next_token)->type != REDIRECT_RIGHT_TOKEN))
		    throw_error("ERROR: Word followed by illegal token");
		    
		    if ((current_token->next_token)->type == REDIRECT_RIGHT_TOKEN)
		      {
			if ((current_token->next_token)->next_token == NULL)
			  throw_error("ERROR: Right redirection empty"); 
			
			if (((current_token->next_token)->next_token)->type != SIMPLE_TOKEN)
			  throw_error("ERROR: Right redirection not followed by a simple token");
			
			current_token = (current_token->next_token)->next_token;
			
			return_command->output = (char*) checked_malloc(sizeof(char)*(current_token->curr.simple_token.word_length));
			
			return_command->output = strncpy((return_command->output),(current_token->curr.simple_token.word_content),(current_token->curr.simple_token.word_length));
			redirect_flag = 1;
		      }
		}
	      if (current_token->next_token != NULL) throw_error("ERROR: Bad command received!");
	    }
	  return return_command;
	}
      else current_token = current_token->next_token;
    }
  return return_command; // If it ever gets here - it's an empty command!
}




command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  token *first_token = make_token_stream (get_next_byte, get_next_byte_argument);
  while((first_token != NULL) && (first_token->type == NEWLINE_TOKEN || first_token->type == SEQUENCE_TOKEN))
    {
      token* delete_token = first_token;
      if (first_token->next_token != NULL)
	(first_token->next_token)->prev_token = NULL;
      
      first_token = first_token->next_token;
      free(delete_token);
    }
  clean_token_stream(first_token);
 
  /*
  // !!! TEST TOKENIZER CODE !!! (remove before submission)
  token *current_token = first_token;
  while(current_token != NULL)
    {
      token_type_printer(current_token);
      token *temp_token = current_token->next_token;
      current_token = temp_token;
    }
  printf("Tokenizing complete\n\n\n");
  // !!! END TEST CODE !!!
  */

  int num_commands;
  token** tokenized_command_array = complete_command_divider(first_token, &num_commands);

  // !!! TEST TOKENIZED COMMAND ARRAY CODE !!! (remove before submission)
  int i = 0;
  for (i; i < num_commands; i++)
    {
      token *current_token = tokenized_command_array[i];
      printf("\n\n *** COMMAND %d *** \n", (i+1));
      while(current_token != NULL)
	{
	  token_type_printer(current_token);
	  token *temp_token = current_token->next_token;
	  current_token = temp_token;
	}
    }
  printf("Tokenized command array created\n\n\n");
  // !!! END TEST CODE !!!


  // Create commands from tokens
  int j = 0;


  command_t* command_list = (command_t*) checked_malloc(sizeof(command_t)*num_commands);

  for (j; j < num_commands; j++)
    {
      printf("<<-----COMMMAND #%d----->\n", (j+1)); //------------------------------------------------------
      command_list[j] = make_single_command(tokenized_command_array[j]);
      printf("<<-----LASTCOMP #%d----->\n", (j+1)); //------------------------------------------------------
    }

  command_stream_t created_commands = (command_stream_t) checked_malloc(sizeof(struct command_stream));
  created_commands->command_array = command_list;
  created_commands->num_commands = num_commands;
  printf("Tokenized command array created\n\n\n");

  /*
  // DEALLOCATE (III) HERE - TOKENIZED COMMANDS
  free(tokenized_command_array);
  
  // DEALLOCATE (I) HERE - TOKENIZER
  token *current_delete_token = first_token;
  while (current_delete_token != NULL)
    {
      token *temp_token = current_delete_token->next_token;
      if (current_delete_token->type == SIMPLE_TOKEN) free(current_delete_token->curr.simple_token.word_content);
      free(current_delete_token);
      current_delete_token = temp_token;
    }
  */
  return created_commands;
}

command_t
read_command_stream (command_stream_t s)
{
  if (s == NULL) return 0;
  else
    {
      if (s->num_commands == 0) return 0;
      else 
	{
	  command_t return_command = s->command_array[0];
	  int i = 0;
	  for (i; i < (s->num_commands)-1; i++)
	    {
	      s->command_array[i] = s->command_array[i+1];
	    }
	  //free(s->commands_array[(s->num_commands)-1]);
	  s->num_commands--;
	  return return_command;
	}
    }
  return 0;
}
