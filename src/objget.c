#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <openssl/evp.h>
#include "utilities.h"
#include "cryputil.h"

int main(int argc, char **argv) {

    char *passphrase;
    int c;
    int err = 0;
    opterr = 0;
    char *options = "k:";

    //Parse options
    while ((c = getopt(argc, argv, options)) != -1) {
        switch (c) {
          case 'k':
            passphrase = optarg;
            break;
          case '?':
            err = 1;
            break;
          default:
            fprintf(stderr, "getopt error\n");
        }
    }

    if(argc!=4 || err==1) {
        fprintf(stderr, "Usage: objget -k <passphrase> <objname>\n");
        return 1;
    }

    char objectname[strlen(argv[3])];
    sprintf(objectname, "%s", argv[3]);

    char *obj = getobj(argv[3], 'r');
    if(obj==NULL) {
        return 0;
    }

    //write file to stdout
    FILE *file = fopen(obj, "r");
    int size = 1024;
    char buffer[size];
    int bytes;

    int i = 0;
    int total = 0;
    char *ciphertext = malloc(++i*size*sizeof(char));
    char *dest;

    while(!feof(file)) {
        bytes = fread(buffer, 1, sizeof(buffer), file);
        
        if(i>1) {
            char *temp = ciphertext;
            char *ciphertext = malloc(i*size*sizeof(char));
            memcpy(ciphertext, temp, total);
            free(temp);
            dest = ciphertext + total;
        }
        else {
            dest = ciphertext;
        }
        
        memcpy(dest, buffer, bytes);
        total+=bytes;
        i++;
    }

    int plaintext_len;

    char *plaintext = decrypt_data(ciphertext, total, passphrase, objectname, &plaintext_len);

    if(plaintext==NULL) {
        printf("%s\n", "Incorrect Passphrase!");
        return 0;
    }

    /* Write file to stdout */
    fwrite(plaintext, 1, plaintext_len, stdout);
    fflush(stdout);

    if(plaintext[plaintext_len-1] != '\n') {
        fprintf(stdout, "\n");
    }

    fclose(file);
    free(obj);
    free(plaintext);
    return 0;
}
