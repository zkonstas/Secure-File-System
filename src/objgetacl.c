#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utilities.h"

int main(int argc, char **argv) {

    if(argc!=2) {
        fprintf(stderr, "Usage: objgetacl <objname>\n");
        return 1;
    }

    char *obj = getobj(argv[1], 'v');
    if(obj == NULL) {
        return 0;
    }

    //If progam arrives here, it means that everything is valid and the user has permission to view the acl
    char *objname = strrchr(obj, '/')+1;
    char *acl = getacl(objname);
    
    char line[1024];
    FILE *aclfile = fopen(acl, "r");

    while(fgets(line, sizeof(line), aclfile) !=NULL) {
        fputs(line, stdout);
    }

    //Check if last line had the \n
    if( line[strlen(line)-1] != '\n') {
        fputs("\n", stdout);
    }

    fclose(aclfile);
    free(obj);
    return 0;
}
