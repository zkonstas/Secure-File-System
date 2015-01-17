# Secure-File-System
A secure file system in C supporting multiple users, access control lists and encryption.

## Overview

### Description

This project is an implementation of a secure file system to illustrate basic security mechanisms used in most commercial software. The basic functionality of the system supports the secure storage and retrieval of objects (files of any type) for all users of the operating system. Additional, more powerful access control is provided by assigning access control lists to objects. Finally, all objects are encrypted using the openssl library and are accessed through a passphrase.

### Commands
The system supports six command as explained below:

**objput obj -k passphrase**
Read an object from stdin and store it using the passphrase *passphrase*

**objget obj -k passphrase**
Retrieve an object *obj* using the passphrase "passphrase and output it to stdout

**objlist [-l]**
List the objects the current user has created. Supplying the optional *-l* parameter displays also the size of each object

**objsetacl obj**
Read an access control list from stdin set it to the object *obj* (replacing the old one)

**objgetacl obj**
Retrieve the current access control list of the object *obj* and write it to stdout

**objtestacl -a access obj**
Test weather the current user has some "access" (rwxpv) for the object *obj*. The command outputs "allowed" or "denied" to stdout based on if the supplied kind of access is permitted

The objsetacl command reads an ACL from stdin.  The form is identical
to that discussed in class: 0 or lines of the following syntax.

	user.group    ops

The possible "ops" are r, w, x, p, and v.  "p" is "permissions";
you need "p" permission to change an ACL.  "v" is "view"; you must
have "v" permission to view an ACL for a file. 

All commands must have a valid username.  When an object is created, the
creator's username is stored (somewhere, somehow).  You are welcome to create
a small number of system users.  Each user has their own namespace; two
different users can create objects of the same name.

objput reads the contents of the object from stdin; objget writes the
contents of a retrieved object to stdout.  objlist lists all of the objects
belonging to the specified user; if the -l option is given (and options
can appear in any order), all metadata associated with the object is
displayed as well.  At this point, that's just the size of the object,
but that will change...

### System Architecture and Access Control

emphasize how the architecture is around access control

There are multiple user and groups.  The list of users and what groups
they are in are also stored as objects.  The userfile is composed of lines in this format:

	username group1 group2 ...

Usernames, group names, and object names can contain letters, digits, and
underscores; no other characters are legal.


For all of these commands, the -u username and -g groupname are the
values to be compared against the ACL when deciding if permission
should be granted or denied.  You will need to add a -g option to
objget and objput; those commands should also check permissions.

Pgm0 specified that each user had a separate name space.  Here,
though, you want to be able to reference other users' objects.
Accordingly, objname needs an extended syntax: you can refer to a
different user's objects via

	username+objname

The heart of this assignment is, of course, the permission-checking.

The list of legal user and group names can be stored in a simple, static
file.

Implementation hint 1: It is perfectly reasonable to store objects
in a single directory with a combination of the user name and object
name as the filename.  It is also reasonable to have a subdirectory
for each user, and store the objects there.  Your choice; both work.

Implementation hint 2: If you choose to use other objects to store
ACLs, remember that these must be objects, not files.  More
specifically, any code you use to read or write them must go through
the object store routines.  Look at it this way -- what you're building
is a simplified file system.  If you want to store something -- like
an ACL -- you have to store it as something you can read: an object,
not a file.  If you choose to do things this way, your internal code
must call the "read object" routine.

Implemenation hint 3: For simplicity, when a new object is created
by some user u, create an initial ACL of

	u.*     rwxpv


Programming with Privileges

	For this assignment, modify the programs for the last
	assignment to use the actual user and group ID.  That is,
	instead of having -u and -g, see what permissions the
	programs are actually running with.

	The catch: you MUST prevent any access to the repository
	that bypasses the ACLs.  This implies that the repository
	must be protected using OS permissions, which in turn means
	that a separate UID is needed.  The programs must either
	be setUID or use message-passing; that's your choice.

	The objects for user and group names are no longer needed;
	use the system's equivalents.

	You may not use the OS ACL mechanisms; you must implement
	them yourself.

	One important design decision: what UID are you using, root
	or some non-privileged one?  Your README file MUST justify
	your choice.  A correct choice with an incorrect justification
	will lose points.  (Of course, an incorrect choice will
	also lose points.)

	You should run this assignment using a VMware virtual
	machine; see the instructions on the web page.

	If you choose to use message-passing, you need not solve
	the start-up problem; it is acceptable to start the daemon
	manually.

	You must supply a script that will create an empty repository,
	including setting all permissions you rely on for correct,
	secure behavior.  The README file should explain the rationale
	for whatever permissions you choose.

	You may (probably will...) find the getuid() and getpwuid()
	routines (and the corresponding routines for groups) to be
	helpful.

	Also: because a lot of the testing needs to involve stuff running
	as different users, the test script must be run as root.  To run
	something as another user, write

	      su username -c "./objget..."

	      where "username" is whatever username that command should
	      run as.



### Encryption

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

You will likely spend more time understanding how to use the MD5 and
AES routines than writing the actual code.  Plan for that...

## Setup

## Testing




The packet includes a C file for the body of each command. Each command source file is named <command_name>.c

In addition it includes a utilities file "utilities.c" (with corresponding headers file)
The utilities file contains the basic functions that check for the validity
of the input data.

Finally, it includes a cryptography file "cryputil.c" (with corresponding headers file).
This is resposible for:
- encrypting and decrypting input data from memory using the OpenSSL library
- checking weather an decryption using the supplied passphraze is valid. If the passphraze is not valid
no decrypiton is performed.

A default file named "userfile.txt" is included and contains some
sample users belonging to same sample groups.

The file "setup.c" when compiled makes sure that the users and groups specified in the userfile
are valid

Files "acl1", "acl2", "file1", "file2" are used for testing

Finally a script name "envinit.sh" reads the users and groups from the userfile and creates them
in the system.
This script creates a privilleged user "admin". This user belongs to a single group by himself named
also "admin". This user is the only user that will have access to the objects. Therefore:

1) An objects directory is created to store the objects. The dir permissions are set from the script
so that only the admin user can read, write and execute it. All others can't do anything else.

2) Each command executable file has its ownership changed to admin. Only admin will be able to read, write
and execute them. All others have no permissions. In addition each commmand is setUID. In this way each command
will be run with the permissions of admin. In this way each command will have access to the objects directory.

We are NOT using root for setUID but the UID from admin. Admin needs only access to the protected directory
objects. If we were using root, then root would be able to have access on everything on the system which would
not be necessary for our application. Therefore we create a privilaged users with as few privilages so that
our application can work.

-------------------------------------------------------

To build the commands with the default file type:
make exec

To build them with a custom userfile type:
make exec userfile="custom_userfile.txt"

To test the commands: a sample test script has been written in the Makefile. To test, type:
make test

-------------------------------------------------------

Notes on commands:
------------------

A file name "userobjects.txt" is created to store the objects that each user
has. This file is used by objget objput and objlist

Also when trying to create a file that already exists, the file is
overwritten with the new input data and it's acl is reverted back to the default

Whenever an empty acl tries to replace an existing acl, the existing acl is
not changed.

