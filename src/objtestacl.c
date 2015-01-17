#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "utilities.h"

int main(int argc, char **argv) {

	char *acc = NULL;

	int c;
    int err = 0;
    opterr = 0;
	char *options = "a:";

	//Parse options
	while ((c = getopt(argc, argv, options)) != -1) {
        switch (c) {
          case 'a':
          	acc = optarg;
          	break;
          case '?':
            err = 1;
            break;
          default:
            fprintf(stderr, "getopt error\n");
        }
	}

    //Check access validity
    char *p = acc;
    int valid = 1;
    
    if(strlen(acc)>5) {
        valid=0;
    }
    while(*p) {
        if (strchr("rwxpv", *p)==NULL) {
            valid = 0;
            break;
        }
        p++;
    } 

	if(argc!=4 || err==1 || !valid) {
        fprintf(stderr, "Usage: objtestacl -a <access> <objname>\n");
        return 1;
    }

    char *obj = getobj(argv[3], 't');

    if(obj==NULL) {
    	return 0;
    }

    //Get user details
    char *user = getuser();
    char *group = getgroup();

    char *objname = strrchr(obj, '/')+1;
    char *acl = getacl(objname);

    int allowed = 1;
    p = acc;
    while(*p) {
        if(checkaclperm(user, group, acl, *p)!=0) {
            allowed = 0;
        }
        p++;
    }

	if(allowed) {
		printf("allowed\n");
    }
    else {
    	printf("denied\n");
    }
    
    free(acl);
    free(obj);
    return 0;
}
