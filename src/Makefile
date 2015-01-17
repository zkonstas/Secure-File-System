CC = gcc

build: objput objget objlist objsetacl objgetacl objtestacl setup

objput: objput.o utilities.o cryputil.o
	$(CC) objput.o utilities.o cryputil.o -lssl -lcrypto -o objput

objget: objget.o utilities.o cryputil.o
	$(CC) objget.o utilities.o cryputil.o -lssl -lcrypto -o objget

objlist: objlist.o utilities.o

objsetacl: objsetacl.o utilities.o

objgetacl: objgetacl.o utilities.o

objtestacl: objtestacl.o utilities.o

setup: setup.o


.PHONY:	exec
ifneq "$(strip $(userfile))" ""
exec:	build
	@echo init to $(userfile)
	@cp $(userfile) userfile.txt
	@chmod a+x setup
	@chmod a+x envinit.sh
	./setup
	./envinit.sh
else
exec:	build
	@echo default_init
	@chmod a+x setup
	@chmod a+x envinit.sh
	./setup
	./envinit.sh
endif

.PHONY: test
test:
	su u1 -c "cat file1"
	! su u1 -c "./objput doc% < file1"
	! su u1 -c "./objput doc doc2 < file1"
	su u1 -c "./objput -k belkin1 doc < file1"
	su u1 -c "./objget -k belkin doc"
	su u1 -c "./objget -k belkin1 doc"
	su u1 -c "./objput -k cat u2+doc2 < file1"
	su u2 -c "./objget -k cat doc"
	su u2 -c "./objget -k cat u1+doc"
	su u1 -c "cat file2"
	su u1 -c "./objput -k dog doc2 < file2"
	su u1 -c "./objlist"
	su u1 -c "./objlist -l"
	su u2 -c "./objlist"
	su u1 -c "./objgetacl doc"
	su u1 -c "cat acl1"
	su u1 -c "./objsetacl doc < acl1"
	su u2 -c "./objget -k dog u1+doc"
	su u2 -c "./objgetacl u1+doc"
	su u2 -c "./objsetacl u1+doc < acl2"
	su u1 -c "./objsetacl u1+doc < acl2"
	su u1 -c "./objgetacl doc"
	! su u1 -c "./objtestacl -a rxs doc"
	su u1 -c "./objtestacl -a rw doc"
	su u1 -c "./objtestacl -a rp doc"
	! su u2 -c "cat ./objects/u1+doc2"

.PHONY: clean
clean:
	rm -rf *.core *.o init setup objsetacl objgetacl objtestacl objput objget objlist userobjects.txt objects
	userdel admin
	userdel u1
	userdel u2
	groupdel g1
	groupdel g3
	groupdel g4
	groupdel g5
	@clear
