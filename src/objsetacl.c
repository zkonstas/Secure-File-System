#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utilities.h"

int main(int argc, char **argv) {

    if(argc!=2) {
        fprintf(stderr, "Usage: objsetacl <objname>\n");
        return 1;
    }

    char *obj = getobj(argv[1], 'p');
    if(obj == NULL) {
        return 0;
    }

    char line[1024];
    int haslines = 0;

    //read the acl from stdin and save id to a temp file: @user+@
    char *objname = strrchr(obj, '/')+1;
    char *acl = getacl(objname);

    char temp[strlen(acl)];
    sprintf(temp, "%s", acl);

    char *aclobj = strrchr(temp, '+')+1;
    sprintf(aclobj, "@");

    FILE *newaclfile = fopen(temp, "w");

    while(fgets(line, sizeof(line), stdin) !=NULL) {
        haslines = 1;
        fputs(line, newaclfile);
    }

    //If the acl file had zero lines do not alter acl.
    if(haslines==0) {
            fprintf(stderr, "ACL file has zero lines! Object ACL not changed!\n");
            fclose(newaclfile);
            free(obj);
            return 0;
    }
    fclose(newaclfile);

    newaclfile = fopen(temp, "r");

    //Read new acl and make sure it is valid
    while(fgets(line, sizeof(line), stdin) !=NULL) {

        char *sep = ". \t\r\n";
        char *user = strtok(line, sep);
        char *group = strtok(NULL, sep);
        char *perm = strtok(NULL, sep);

        if(user==NULL || group==NULL || perm==NULL || strtok(NULL, sep)!=NULL) {
            fprintf(stderr, "Invalid input ACL file! Object ACL not changed!\n");
            fclose(newaclfile);
            free(obj);
            return 0;
        }

        //check that user, group and permissions are valid
        if(checkvalidpermissions(user, group, perm)!=0) {
            fprintf(stderr, "Users/groups in new ACL file not legal/valid or invalid permissions! Object ACL not changed!\n");
            fclose(newaclfile);
            free(obj);
            return 0;
        }

    }
    fclose(newaclfile);

    //If progam arrives here, it means that everything is valid so the new acl can replace the old one
    //We do this by deleting the old acl and renaming the newaclfile with the name of the old
    remove(acl);
    rename(temp, acl);

    fprintf(stderr, "Object ACL changed successfully!\n");
    
    free(acl);
    free(obj);
    return 0;
}
