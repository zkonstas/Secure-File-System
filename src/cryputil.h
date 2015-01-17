#ifndef _CRYPUTIL_H_
#define _CRYPUTIL_H_

char *encrypt_data(char *plaintext, int size, char *passphrase, char *name, int *cipher_len);

char *decrypt_data(char *ciphertext, int size, char *passphrase, char *name, int *plain_len);

#endif