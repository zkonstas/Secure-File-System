#!/bin/sh

#create admin user
useradd admin
echo created user admin

#create objects directory to store objects and assign ownership only to admin. setguid so that objects
#created in the directory can inherit the group of objects which is admin
umask 077
mkdir objects
echo created objects directory to store objects
chown admin objects
chgrp admin objects
chmod 2700 objects
echo assigned objects ownership only to admin

#setuid to admin for each of the commands. allow everyone to execute the files
chown admin objput objget objlist objsetacl objgetacl objtestacl
chgrp admin objput objget objlist objsetacl objgetacl objtestacl
chmod 4101 objput objget objlist objsetacl objgetacl objtestacl

#remove permissions for setup from others
chmod o-rwx setup

#create users based on input file
while read line
do
	isuser=true
	userexists=false

	for token in $line
	do
		if $isuser ; then
			user=$token
			isuser=false

			#check if user exists
			if grep -q $user /etc/passwd ; then
				userexists=true
			fi
		else
			#check if group exists and create it if it does not
			if ! grep -q $token /etc/group ; then
				groupadd $token
				echo added group $token
			fi
			#add a new user to an existing group
			if ! $userexists ; then
				useradd -g $token $user
				userexists=true
				echo added new user $user to existing group $token
			else
				#add an existing user to an existing group
				usermod -a -G $token $user
				echo added existing user $user to existing group $token
			fi
		fi
	done
done < userfile.txt