#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "utilities.h"

int main(int argc, char **argv) {
    extern char *objects;

    int c;
    int err = 0;
    opterr = 0;
    
    char *options = "l";
    int l = 0;

    while ((c = getopt(argc, argv, options)) != -1) {
        switch (c) {
          case 'l':
            l = 1;
            break;
          case '?':
            err =1;
            break;
          default:
            fprintf(stderr, "getopt error\n");
        }
    }
    
    if(argc > 2 || err==1) {
        fprintf(stderr, "Usage: objlist [-l]\n");
        return 1;
    }

    char *user = getuser();

    char line[1024];
    char *userobjectsname = "userobjects.txt";
    char userobjects[strlen(objects)+strlen(userobjectsname)];
    strcpy(userobjects, objects);
    strcat(userobjects, userobjectsname);

    FILE *usrobjfile = fopen(userobjects, "r");

    if(usrobjfile==NULL) {
        return 0;
    }

    while(fgets(line, sizeof(line), usrobjfile) !=NULL) {
        char *sep = " \t\r\n";
        char *usr = strtok(line, sep);
        char *obj = NULL;
        long size = 0;

        if(strcmp(usr, user)==0) {

            while( (obj = strtok(NULL, sep))!=NULL) {

                //If -l option is on, read the corresponding size of the object
                if(l==1) {
                    char *filename = malloc(strlen(objects)+strlen(usr)+strlen(obj)+1);
                    sprintf(filename, "%s%s+%s", objects, usr, obj);

                    FILE *file = fopen(filename, "rb");
                    fseek(file, 0, SEEK_END);
                    size = ftell(file);
                    printf("%s %lu bytes\n", obj, size);
                }
                else {
                    printf("%s\n", obj);                    
                }
            }
        }
    }

    fclose(usrobjfile);
    return 0;
}
