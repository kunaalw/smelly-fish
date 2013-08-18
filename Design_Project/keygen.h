#include <stdio.h>
#include <string.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

 
#define KEY_LENGTH 2048
#define KEY_EXPONENT 5


// Struct to hold key info for a given peer
typedef struct key_chain 
{
  int key_length;
  int key_exponent;
  int max_chars;

  int size_private;
  int size_public;

  char* key_private;
  char* key_public;
} 
  key_chain;

key_chain keygen();
