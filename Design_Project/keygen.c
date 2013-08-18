#include "keygen.h"

 
#define KEY_LENGTH 2048
#define KEY_EXPONENT 5


key_chain keygen()
{
  // Key-chain initialization
  key_chain ret_pair;
  ret_pair.key_length = KEY_LENGTH;
  ret_pair.key_exponent = KEY_EXPONENT;
  ret_pair.max_chars = KEY_LENGTH/8;

  // Generate key-pair
  RSA* key_pair = RSA_generate_key(ret_pair.key_length, ret_pair.key_exponent, NULL, NULL);
  
  BIO *bio_private = BIO_new(BIO_s_mem());
  BIO *bio_public = BIO_new(BIO_s_mem());
 
  PEM_write_bio_RSAPrivateKey(bio_private, key_pair, NULL, NULL, 0, NULL, NULL);
  PEM_write_bio_RSAPublicKey(bio_public, key_pair);

  ret_pair.size_private = BIO_pending(bio_private);
  ret_pair.size_public = BIO_pending(bio_public);

  ret_pair.key_private = malloc(ret_pair.size_private+1);
  ret_pair.key_public = malloc(ret_pair.size_public+1);

  BIO_read(bio_private, ret_pair.key_private, ret_pair.size_private);
  ret_pair.key_private[ret_pair.size_private] = '\0';
  BIO_read(bio_public, ret_pair.key_public, ret_pair.size_public);
  ret_pair.key_public[ret_pair.size_public] = '\0';
  
  return ret_pair;
}
