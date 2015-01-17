#ifndef _UTILITIES_H_
#define _UTILITIES_H_

char *getobj(char *objname, char perm);

char *getfullobjname(char *objname, char *username);

char *getuser();

char *getgroup();

char *getacl(char *objname);

int validname(char *name);

int legaluser(char *user);

int legalgroup(char *group);

int checkaclperm(char *user, char *group, char *acl, char perm);

int checkvalidpermissions(char *user, char *group, char *perm);

#endif
