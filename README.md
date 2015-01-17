# Secure-File-System
A secure file system in C supporting multiple users, access control lists and encryption.

## Description

This project is an implementation of a secure file system to illustrate basic security mechanisms used in most commercial pieces of software. The basic functionality of the system supports the secure storage and retrieval of objects (files of any type) for all users of the operating system. Additional, more powerful access control is provided by assigning access control lists to objects. Finally, all objects are encrypted using the openssl library and can be accessed through a passphrase.

## Commands
The system supports six command as explained below:

	objput obj -k passphrase
Read the contents of an object *obj* from stdin and store the object using the passphrase *passphrase*

`objget obj -k passphrase`: Retrieve an object *obj* using the valid passphrase *passphrase* and write its contents to stdout

`objlist [-l]`: List all of the objects belonging to the current user. If the optional *-l* parameter is given then the size of each object is also displayed

`objsetacl obj`: Read an access control list from stdin and set it to the object *obj* (replacing the old one)

`objgetacl obj`:  Retrieve the current access control list of the object *obj* and write it to stdout

`objtestacl -a access obj`: Test weather the current user has the supplied *access* (any combination of *rwxpv*) for the object *obj*. The command outputs "allowed" or "denied" to stdout based on if the supplied kind of access is permitted

### Details

- When trying to create a file that already exists, the file is overwritten with the new input data and it's acl is reverted back to the default.
- Whenever an empty acl tries to replace an existing acl, the existing acl is not changed.

## System Architecture and Access Control

The system provides access contol to objects for different users. All the users of the operating system can create objects in the "Secure File System" and access them including objects of other users. All objects are stored in the single directory `objects/` that is created by the setup script and have the following sytax:

	username+objectname

Consequently two different users can create objects of the same name. A file name `userobjects.txt` is created to store the information about which objects each user has. When an object is created an entry of the username along with the object name is appended to this file. This file is used by objget objput and objlist.

Access control to files is provided through access control lists enforced by the **Secure File System**. In other words, our system checks the ACL's of each object and whether they current user that is trying to modify it has the required permissions. For this reason unathorized access to the objects through OS's file system is prevented by performing the following two tasks.

1) Creating the repository `objects/` directory to store the objects. Then the directory permissions are changed so that only some privileged user **admin** can read, write and execute it. All others have no permissions to do anything else, including access the objects stored in it.

2) Each command executable file has its ownership changed to **admin**. Only admin will be able to read, write and execute them. All others have no permissions. In addition each commmand is setUID. In this way each command will be run with the permissions of **admin**. In this way each command will have access to the objects directory.

These two tasks are perfomed by the system script `envinit.sh`.

### Access Control Lists Details

Each object has an ACL file with the following syntax:

	@username+objname

Each ACL file has 0 or more lines of the following syntax:

	user.group    ops

The possible **ops** are **r**, **w**, **x**, **p**, and **v**. "r" is read permissions to get an object. "w" is write permissions to overwrite an object. "x" is execute permissios to execute an object. p" is "permissions"; you need "p" permission to change an ACL.  "v" is "view"; you must have "v" permission to view an ACL for a file. 

When a new object is created by some user u, create an initial ACL of

	u.*     rwxpv

## Encryption

The **Secure File System** also supports file encryption. Objects are encrypted when created and decrypted when retrieved. Files are encrypted with AES in CBC mode using a random 128-bit file-encrypting key. This per-file key is encrypted with the user-supplied passphrase. The passphraze is converted to a 128-bit key using MD5 and used as a key to encrypt/decrypt the random per-file key. Finally, each encrypted key for each object is stored securely in the `objects/` directory using the following filename sytax:

	$username+objname


## Setup

To setup your environment including:
- creating the appropriate directories
- setting up the required permissions
- creating the users and groups of the system
- compile all files

you have two options:

1) For creating users/groups supplied with the default userfile run:

	make exec

2) For creating users/groups supplied with a custom userfile run:

	make exec userfile="custom_userfile.txt"

The custom userfile must be composed of 1 or more lines of the following syntax:

	username group1 group2 ...

These commands will validate the userfile, create the appropriate users in the operating system.

## Testing
A sample test script has been written in the Makefile. To test, type:

	make test

## Source Files Description

**<command_name>.c**
The body of each command

**utilities.c** and **utilities.h**
Contains the basic functions that check for the validity of the input data.

**cryputil.c** and **cryputil.h**
This is resposible for:
- encrypting and decrypting input data from memory using the OpenSSL library
- checking weather an decryption using the supplied passphraze is valid. If the passphraze is not valid
no decrypiton is performed

**userfile.txt**
Sample users names and group names used by the setup script to create the corresponding users and groups in the system.

**setup.c**
Verifies that the user and group names specified in the userfile are valid

**acl1, acl2, file1, file2**
Used by the testing script

**envinit.sh**
The setup script that reads the user and group hames from the userfile and creates them in the system.
This script also creates a privilleged user "admin". This user belongs to a single group by himself named
also "admin". This user is the only user that will have access to the objects.

## Additional Details
This system was developed as a project for the class COMS4187 in the Fall 2014 at Columbia University.

