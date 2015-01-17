#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/md5.h>
#include <openssl/aes.h>
#include <string.h>
#include <linux/random.h>
#include "utilities.h"
#include "cryputil.h"

int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
  unsigned char *iv, unsigned char *ciphertext);
int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
  unsigned char *iv, unsigned char *plaintext);
char *getkeys(char *objname);
void printhex(char *data, int len);

extern char *objects;

/* method to destroy a certain part of memory */
void delmem(char *p, int len) {
  int i;
  for(i=0;i<len;i++) {
    *p = '\0';
  }
}

char *encrypt_data(char *plaintext, int size, char *passphrase, char *name, int *cipher_len) {
  /* Initialise the library */
  ERR_load_crypto_strings();
  OpenSSL_add_all_algorithms();
  OPENSSL_config(NULL);

  unsigned char key[16];
  unsigned char key_copy[16];
  unsigned char iv_data[16];
  unsigned char iv_data_copy[16];
  unsigned char iv_key[16];
  unsigned char iv_key_copy[16];

  /* Read random characters for the 2 ivs and key */
  FILE *urandom = fopen("/dev/urandom", "r");
  fread(key, 1, sizeof(key), urandom);
  fread(iv_data, 1, sizeof(iv_data), urandom);
  fread(iv_key, 1, sizeof(iv_key), urandom);

  /* create copies of the keys */
  memcpy(iv_data_copy, iv_data, 16);
  memcpy(iv_key_copy, iv_key, 16);

  int enc_len = ((size + 16) / 16) * 16 + 16;
  unsigned char ciphertext[enc_len];

  /* Encrypt the plaintext */
  int ciphertext_len = encrypt((unsigned char *)plaintext, size, key, iv_data, ciphertext);
  *cipher_len=ciphertext_len;

  /* Get the MD5 hash of the passphrase */
  char md5pass[16];
  MD5((const unsigned char *)passphrase, (unsigned long)strlen(passphrase), (unsigned char *)md5pass);

  /* Encrypt the generated key with the MD5 hash of the passphrase */
  unsigned char cipherkey[32];

  int cipherkey_len = encrypt((unsigned char *)key, 16, (unsigned char *)md5pass, iv_key, cipherkey);

  /* Create an entry: IV_data followed by IV_key followed by the key */
  int total = 32+cipherkey_len;
  char keys[total];
  memcpy(keys, iv_data_copy, 16);
  memcpy(keys+16, iv_key_copy, 16);
  memcpy(keys+32, cipherkey, cipherkey_len);

  /*Save the entry in a file name in the format: $user+objectname */
  char *user = getuser();
  char *objname = getfullobjname(name, user);

  char *keysfilename = getkeys(objname);
  FILE *keysfile = fopen(keysfilename, "w");

  fwrite(keys, 1, total, keysfile);

  if(ferror(keysfile)) {
    fprintf(stderr, "%s\n", "ERROR: Error while writing key to file!");
  }

  /* Clean up */
  EVP_cleanup();
  ERR_free_strings();
  delmem(iv_data, 16);
  delmem(iv_data_copy, 16);
  delmem(iv_key, 16);
  delmem(iv_key_copy, 16);
  delmem(key, 16);
  delmem(keys, total);
  fclose(keysfile);
  fclose(urandom);

  /* Allocate memory for the encrypted text and return the address to the caller */
  char *encryptedtext = malloc(ciphertext_len);
  memcpy(encryptedtext, ciphertext, ciphertext_len);
  return encryptedtext;
}

char *decrypt_data(char *ciphertext, int size, char *passphrase, char *name, int *plain_len) {
  /* Initialise the library */
  ERR_load_crypto_strings();
  OpenSSL_add_all_algorithms();
  OPENSSL_config(NULL);
  
  int ciphertext_len = size;
  char cipherkey[128];
  char key[128];
  char iv_data[16];
  char iv_key[16];

  /*Read the 2 IVs and the encrypted key for the file */
  char *user = getuser();
  char *objname = getfullobjname(name, user);

  char *keysfilename = getkeys(objname);
  FILE *keysfile = fopen(keysfilename, "r");
  
  fread(iv_data, 1, 16, keysfile);
  fread(iv_key, 1, 16, keysfile);
  int cipherkey_len = fread(cipherkey, 1, 32, keysfile);

  if(ferror(keysfile)) {
    fprintf(stderr, "%s\n", "ERROR: Error while reading key from file!");
  }
  
  /* Get the MD5 hash of the passphrase */
  char md5pass[16];
  MD5((const unsigned char *)passphrase, (unsigned long)strlen(passphrase), (unsigned char *)md5pass);

  /*Decrypt the cipherkey */
  int key_len = decrypt((unsigned char *)cipherkey, cipherkey_len, (unsigned char *)md5pass,
    (unsigned char *)iv_key, (unsigned char *)key);

  if(key_len==-1) {
    return NULL;
  }
  
  /* Buffer for the decrypted text */
  unsigned char decryptedtext[ciphertext_len];
  
  /* Decrypt the ciphertext */
  int decryptedtext_len = decrypt((unsigned char *)ciphertext, ciphertext_len, (unsigned char *)key,
    (unsigned char *)iv_data, decryptedtext);
  *plain_len = decryptedtext_len;

  /* Clean up */
  EVP_cleanup();
  ERR_free_strings();
  delmem(iv_data, 16);
  delmem(iv_key, 16);
  delmem(key, 128);

  /* Allocate memory for the decrypted text and return the address to the caller */
  char *plaintext = malloc(decryptedtext_len);
  memcpy(plaintext, decryptedtext, decryptedtext_len);
  return plaintext;
}

void handleErrors(void)
{
  ERR_print_errors_fp(stderr);
}

int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
  unsigned char *iv, unsigned char *ciphertext)
{
  EVP_CIPHER_CTX *ctx;

  int len;

  int ciphertext_len;

  /* Create and initialise the context */
  if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

  /* Initialise the encryption operation. */
  if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv))
    handleErrors();

  /* Provide the message to be encrypted, and obtain the encrypted output.
   * EVP_EncryptUpdate can be called multiple times if necessary
   */
  if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
    handleErrors();
  ciphertext_len = len;

  /* Finalise the encryption. Further ciphertext bytes may be written at
   * this stage.
   */
  if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) handleErrors();
  ciphertext_len += len;

  /* Clean up */
  EVP_CIPHER_CTX_free(ctx);

  return ciphertext_len;
}

int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
  unsigned char *iv, unsigned char *plaintext)
{
  EVP_CIPHER_CTX *ctx;

  int len;

  int plaintext_len;

  /* Create and initialise the context */
  if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

  /* Initialise the decryption operation. */
  if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv))
    handleErrors();

   // Provide the message to be decrypted, and obtain the plaintext output.
   // * EVP_DecryptUpdate can be called multiple times if necessary
   
  if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
    handleErrors();
  
  plaintext_len = len;

  /* Finalise the decryption. Further plaintext bytes may be written at
   * this stage.
   */
  if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) {

    //Check if the reason for the error is bad decryption
    if(ERR_GET_LIB(ERR_peek_last_error())==6 && ERR_GET_FUNC(ERR_peek_last_error())==101
      && ERR_GET_REASON(ERR_peek_last_error())==100 ) {
      
      // fprintf(stderr, "%s\n",ERR_reason_error_string(ERR_peek_last_error()) );  
      return -1;
    }
    else {
      handleErrors();  
    }
  }
  plaintext_len += len;

  /* Clean up */
  EVP_CIPHER_CTX_free(ctx);

  return plaintext_len;
}

char *getkeys(char *objname) {
  char *keys = malloc(strlen(objects)+strlen(objname)+1);
  sprintf(keys, "%s$%s", objects, objname);
  return keys;
}

void printhex(char *data, int len) {
  int i=0;

  for(i=0;i<len;i++) {
    printf("%02x ", data[i]);
  }
  printf("\n");
}
