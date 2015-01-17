#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

int validname(char *name);
int legaluserfile();

char *userfile = "userfile.txt";

int main(int argc, char **argv) {

	//check that the supplied userfile is valid
    if(legaluserfile()!=0) {
        fprintf(stderr, "Userfile contains not valid usernames or groupnames!\n");
        return 0;
    }
	return 0;
}

//It checks whether a user/group name contains only alphanumeric characters and or underscores
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

//It checks whether the usernames and groupnames in the userfile are valid
int legaluserfile() {
	char line[1024];
	char *name;
	char *sep = " \t\r\n";

	FILE *file = fopen(userfile, "r");

	while(fgets(line, sizeof(line), file) !=NULL) {
		name = strtok(line, sep);

		//check if the userfile username is valid
		if(!validname(name)) {
			fclose(file);
			return 1;
		}
		while((name = strtok(NULL, sep) )!= NULL) {

			//check if the userfile group is valid
			if(!validname(name)) {
				fclose(file);
				return 1;
			}
		}
	}
	fclose(file);
	return 0;
}