#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
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
        fprintf(stderr, "Usage: objput -k <passphrase> <objname>\n");
        return 1;
    }

    extern char *objects;

    char objectname[strlen(argv[3])];
    sprintf(objectname, "%s", argv[3]);

    char *obj = getobj(argv[3], 'w');
    if(obj==NULL) {
        return 0;
    }

    //read stdin and save it to memory
    int size = 1024;
    char line[size];
    char buffer[size];
    int bytes;
    int i = 0;
    int total = 0;
    char *plaintext = malloc(++i*size*sizeof(char));
    
    char *dest = plaintext;

    while((bytes = fread(buffer, 1, size, stdin)) > 0) {

        if(i>1) {
            char *temp = plaintext;
            char *plaintext = malloc(i*size*sizeof(char));
            memcpy(plaintext, temp, total);
            free(temp);
        }

        dest = plaintext + total;
        memcpy(dest, buffer, bytes);
        total+=bytes;
        i++;
    }

    /* Check for errors */
    if(ferror(stdin) || !feof(stdin)) {
        fprintf(stderr, "ERROR while reading file from stdin");
    }

    /*make sure messages display properly */
    if(plaintext[total-1]!='\n') {
        fprintf(stderr, "\n");
    }

    int ciphertext_len;

    //encrypt text and write to file
    char *ciphertext = encrypt_data(plaintext, total, passphrase, objectname, &ciphertext_len);

    FILE *file = fopen(obj, "w");
    fwrite(ciphertext, 1, ciphertext_len, file);
    fclose(file);

    //create default acl for file
    char *objname = strrchr(obj, '/')+1;
    char *acl = getacl(objname);

    //check if the object exists or not
    FILE *f = fopen(acl, "r");
    if(f==NULL) {

        char object[strlen(objname)];
        strcpy(object, objname);

        char *usr = strtok(object, "+");
        char *objusr = strtok(NULL, "+");

        //set all permission for user putting the object
        sprintf(line, "%s.* rwxpv\n", usr);

        FILE *aclfile = fopen(acl, "w");
        fputs(line, aclfile);
        fclose(aclfile);

        //Add the object to the user userobjects if it does not already exist
        char *userobjectsname = "userobjects.txt";
        char userobjects[strlen(objects)+strlen(userobjectsname)];
        strcpy(userobjects, objects);
        strcat(userobjects, userobjectsname);

        FILE *usrobjfile = fopen(userobjects, "a+");
        int objsaved = 0;

        while(fgets(line, sizeof(line), usrobjfile) !=NULL) {
            char *sep = " \t\r\n";
            char *name = strtok(line, sep);

            if(strcmp(name, usr)==0) {

                while( (name = strtok(NULL, sep))!=NULL) {

                    if(strcmp(name, objusr)==0) {
                        objsaved = 1;
                    }
                }
            }
        }

        if(objsaved==0) {
            sprintf(line, "%s %s\n", usr, objusr);
            fputs(line, usrobjfile);
        }
        fclose(usrobjfile);
        fprintf(stderr, "Object %s successfully created!\n", objname);
    }
    else {
        fclose(f);
        fprintf(stderr, "Object %s successfully overwritten!\n", objname);    
    }

    /* Clean Up */
    free(acl);
    free(obj);
    free(ciphertext);
    return 0;
}
