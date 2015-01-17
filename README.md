# Secure-File-System
A secure file system in C supporting multiple users, access control lists and encryption.

## Description

This project is an implementation of a secure file system to illustrate basic security mechanisms used in most commercial pieces of software. The basic functionality of the system supports the secure storage and retrieval of objects (files of any type) for all users of the operating system. Additional, more powerful access control is provided by assigning access control lists to objects. Finally, all objects are encrypted using the openssl library and can be accessed through a passphrase.

## Commands
The system supports six command as explained below:

`objput obj -k passphrase`: Read the contents of an object *obj* from stdin and store the object using the passphrase *passphrase*

`objget obj -k passphrase`: Retrieve an object *obj* using the valid passphrase *passphrase* and write its contents to stdout

`objlist [-l]`: List all of the objects belonging to the current user. If the optional *-l* parameter is given then the size of each object is also displayed

`objsetacl obj`: Read an access control list from stdin and set it to the object *obj* (replacing the old one)

`objgetacl obj`:  Retrieve the current access control list of the object *obj* and write it to stdout

`objtestacl -a access obj`: Test weather the current user has the supplied *access* (any combination of *rwxpv*) for the object *obj*. The command outputs "allowed" or "denied" to stdout based on if the supplied kind of access is permitted

### Details

- When trying to create a file that already exists, the file is overwritten with the new input data and it's acl is reverted back to the default.
- Whenever an empty acl tries to replace an existing acl, the existing acl is not changed.

## System Architecture and Access Control

Objects are stored in a single directory with the following syntax:
username+objname

acls: @username+objname


When an object is created, the
creator's username is stored (somewhere, somehow).  You are welcome to create
a small number of system users.  Each user has their own namespace; two
different users can create objects of the same name.

acl form: 0 or lines of the following syntax.

	user.group    ops

The possible "ops" are r, w, x, p, and v.  "p" is "permissions";
you need "p" permission to change an ACL.  "v" is "view"; you must
have "v" permission to view an ACL for a file. 

emphasize how the architecture is around access control

There are multiple user and groups.  The list of users and what groups
they are in are also stored as objects.  The userfile is composed of lines in this format:

	username group1 group2 ...

Usernames, group names, and object names can contain letters, digits, and
underscores; no other characters are legal.

Pgm0 specified that each user had a separate name space.  Here,
though, you want to be able to reference other users' objects.
Accordingly, objname needs an extended syntax: you can refer to a
different user's objects via

The heart of this assignment is, of course, the permission-checking.

when a new object is created by some user u, create an initial ACL of

	u.*     rwxpv


Programming with Privileges

	For this assignment, modify the programs for the last
	assignment to use the actual user and group ID.  That is,
	instead of having -u and -g, see what permissions the
	programs are actually running with.

This implies that the repository must be protected using OS permissions, which in turn means that a separate UID is needed.  The programs must either be setUID

1) An objects directory is created to store the objects. The dir permissions are set from the script
so that only the admin user can read, write and execute it. All others can't do anything else.

2) Each command executable file has its ownership changed to admin. Only admin will be able to read, write
and execute them. All others have no permissions. In addition each commmand is setUID. In this way each command
will be run with the permissions of admin. In this way each command will have access to the objects directory.

We are NOT using root for setUID but the UID from admin. Admin needs only access to the protected directory
objects. If we were using root, then root would be able to have access on everything on the system which would
not be necessary for our application. Therefore we create a privilaged users with as few privilages so that
our application can work.

A file name "userobjects.txt" is created to store the objects that each user
has. This file is used by objget objput and objlist

## Encryption

File Encryption and Key Storage

For this assignment, add a mechanism to encrypt objects when they're
created, and to decrypt objects when they're retrieved.  Again, use
your VM; this is in addition to the setuid features you've already added.

All files must be encrypted with a random file-encrypting key.  Read
enough bytes for the key from /dev/urandom.  This per-file key must be
encrypted with a user-supplied passphrase.  To do this, add an option

	-k passphrase

to the objget and objput commands.  As mentioned in class, putting a
passphrase on the command line is a bad idea; I specify it here to
simplify testing.

Convert the passphrase to a 128-bit key using MD5.  MD5 is available
as part of the openssl library; You'll also need to use '-lssl'
when linking your program.  (There are also reasons why this isn't
a great way to create a key from a passphrase, but they're beyond
the scope of this class.)

Because you're encrypting with a random key, you have to store this
encrypted key somewhere.  It's up to you to pick a suitable place.
VERY IMPORTANT: the setuid program should verify the decryption--you
get to decide how that check is done--before returning anything.
The encrypted file-encrypting key should NOT be available; otherwise,
an attacker could launch an offline password-guessing attack.

Use AES with 128-bit keys and CBC mode.

For documentation on the EVP interface to SSL, see
	
	http://wiki.openssl.org/index.php/Libcrypto_API
	http://wiki.openssl.org/index.php/EVP
	http://wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption

Remember that AES in CBC mode requires an IV and can only encrypt multiples
of 16 bytes.  Your code must cope with this -- any file length, including
0 bytes, must be supported.

## Setup
You must supply a script that will create an empty repository, including setting all permissions you rely on for correct, secure behavior.

To build the commands and setup your environment with the default userfile supplied:
make exec

To build the commands and setup your environment with a custom userfile type:
make exec userfile="custom_userfile.txt"

## Testing
To test the commands: a sample test script has been written in the Makefile. To test, type:
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

