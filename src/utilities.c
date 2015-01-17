#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include "utilities.h"

char *userfile = "userfile.txt";
char *objects = "./objects/";
char *acl = "./acl/";

char *getobj(char *name, char perm) {

	//get info about the user running this process. Get the name for the user and its group
    char *user = getuser();
    char *group = getgroup();

    char *objname = getfullobjname(name, user);

    //Check if objname is valid
    if(objname==NULL) {
        fprintf(stderr, "Objname must be in the form <username>+<objname> or <objname> with only alphanumeric characters or underscores!\n");
        return NULL;
    }

    char *acl = getacl(objname);

    //Create location of object
    char *obj = malloc(strlen(objects)+strlen(objname));
    sprintf(obj, "%s%s", objects,objname);
    free(objname);

    //Test weather the file exists by trying to open the acl of the file
    FILE *aclfile = fopen(acl, "r");

    if(aclfile==NULL) {
    	
    	if(perm=='w') {
    		//verify that the user is creating a new object in his own namespace
    		char *objname = strrchr(obj, '/')+1;
    		char temp[strlen(objname)];
    		sprintf(temp, "%s", objname);

    		char *foruser = strtok(temp, "+");

    		if(strcmp(foruser, user)==0) {
    			return obj;	
    		}
    		else {
				fprintf(stderr, "You can't put an object in another user's namespace!\n");
        		free(obj);
        		return NULL;	
    		}
    	}
    	else {
    		fprintf(stderr, "The object you are trying to access does not exist!\n");
        	free(obj);
        	return NULL;	
    	}
    }
    else {
    	fclose(aclfile);
    }

    //In the case of objtestacl we do not need to check for permissions
    if(perm=='t') {
    	return obj;
    }

    //We check if we have the required permission to perform the requested operation on the object
    if(checkaclperm(user, group, acl, perm)!=0) {
    	if(perm=='p') {
    		fprintf(stderr, "You do not have permission to change the ACL for this object!\n");	
    	}
    	else if(perm=='v') {
    		fprintf(stderr, "You do not have permission to view the ACL for this object!\n");
    	}
    	else if(perm=='w') {
    		fprintf(stderr, "You do not have permission to write to this object!\n");
    	}
    	else if(perm=='r') {
    		fprintf(stderr, "You do not have permission to read this object!\n");
    	}
        free(obj);
        return NULL;
    }
    
	return obj;
}

int validname(char *name) {

	int len = strlen(name);

	int i;
	for(i=0;i<len;i++) {
		if(!isalnum(name[i]) && name[i]!='_') {
			return 0;
		}
	}
    return 1;
}

char *getfullobjname(char *objname, char *username) {
	char *usr = strtok(objname, "+");
	char *obj = strtok(NULL, "+");

	//check if the user supplied only objname. In this case his username is implicitely added in front
	if(obj==NULL) {
		obj = usr;
		usr = username;
	}

	if(!validname(usr) || !validname(obj) || strtok(NULL, "+")!=NULL) {
		return NULL;
	}

	char *copy = malloc(strlen(usr)+strlen(obj)+1);
    sprintf(copy, "%s+%s", usr, obj);
    return copy;
}

char *getacl(char *objname) {
	char *acl = malloc(strlen(objects)+strlen(objname)+1);
	sprintf(acl, "%s@%s", objects, objname);
	return acl;
}

char *getuser() {
	int uid = getuid();
    struct passwd *usr = getpwuid(uid);

    return usr->pw_name;;
}

char *getgroup() {
	int uid = getuid();
	struct passwd *usr = getpwuid(uid);
    struct group *grp = getgrgid(usr->pw_gid);

    return grp->gr_name;
}

int legaluser(char *user) {
	errno = 0;

	struct passwd *usr = getpwnam(user);

	if(errno==0 && usr==NULL) {
		return 1;
	}
	else if(errno!=0) {
		fprintf(stderr, "getpwnam errno: %d\n", errno);
		return 1;
	}
	return 0;
}

int legalgroup(char *group) {
	errno = 0;

	struct group *grp = getgrnam(group);

	if(errno==0 && grp==NULL) {
		return 1;
	}
	else if(errno!=0) {
		fprintf(stderr, "getgrnam errno: %d\n", errno);
		return 1;
	}
	return 0;

}

int checkaclperm(char *user, char *group, char *acl, char perm) {
	char *usr = NULL;
	char *grp = NULL;
	char *permissions = NULL;
	char line[1024];
	char *sep = ". \t\r\n";

	FILE *aclfile = fopen(acl, "r");

	while(fgets(line, sizeof(line), aclfile) !=NULL) {
		usr = strtok(line, sep);
		grp = strtok(NULL, sep);
		permissions = strtok(NULL, sep);

		//check if there is a user permission and validate it
		if(strcmp(usr, user)==0) {
			if( strchr(permissions, perm)!=NULL ) {
				fclose(aclfile);
				return 0;
			}
		}

		//Check if there is a group permission, get all the users of the group and validate it
		if(strcmp(grp, "*")!=0) {
			struct group *aclgroup = getgrnam(grp);

			char **members = aclgroup->gr_mem;

  			while (*members) {
      			if( strchr(permissions, perm)!=NULL ) {
					fclose(aclfile);
					return 0;
				}
      			members++;
    		}
		}

		//check for the everyone else case
		if(strcmp(usr, "*")==0 && strcmp(grp, "*")==0) {
			if( strchr(permissions, perm)!=NULL ) {
				fclose(aclfile);
				return 0;
			}
		}
	}
	fclose(aclfile);
	return 1;
}

int checkvalidpermissions(char *user, char *group, char *perm) {

	//Check that user is legal/valid  
	if(strcmp(user, "*")!=0 && legaluser(user)!=0) {
		return 1;
	}
	//check group is legal/valid
	if(strcmp(group, "*")!=0 && legalgroup(group)!=0) {
		return 1;
	}

	//We map an integer array to rwxpv. We flip flags to 1 for each permission that we find
	int permissions[5];

	int i;
	for(i=0;i<5;i++){
		permissions[i]=0;
	}

	for(i=0;i<strlen(perm);i++) {

		switch (perm[i]) {
			case 'r':
				if(permissions[0]!=0) {
					return 1;
				}
				permissions[0]=1;
			  	break;
			case 'w':
			  	if(permissions[1]!=0) {
					return 1;
				}
				permissions[1]=1;
			  	break;
			case 'x':
			  	if(permissions[2]!=0) {
					return 1;
				}
				permissions[2]=1;
			  	break;
			case 'p':
			  	if(permissions[3]!=0) {
					return 1;
				}
				permissions[3]=1;
			  	break;
			case 'v':
			  	if(permissions[4]!=0) {
					return 1;
				}
				permissions[4]=1;
			  	break;
			default:
			  	return 1;
			  	break;
		}
	}
	return 0;
}
