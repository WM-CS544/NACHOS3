# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# if you are cross-compiling, you need to point to the right executables
# and change the flags to ld and the build procedure for as
GCCDIR = /home/f85/kearns/nachos_compiler/bin/
#LDFLAGS = -T script -n -nostartfiles
LDFLAGS = -T nachos.ld -n -nostartfiles
ASFLAGS =
CPPFLAGS = $(INCDIR) -DCHANGED


# if you aren't cross-compiling:
#GCCDIR =
#LDFLAGS = -N -T 0
#ASFLAGS =
#CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)decstation-ultrix-gcc
AS = $(GCCDIR)decstation-ultrix-as
LD = $(GCCDIR)decstation-ultrix-ld

CPP = /usr/bin/cpp -E
INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR) -DCHANGED -nostartfiles

all: halt shell matmult sort fileio fromcons hellocons hellofile newfork parent_child kid \
	seq_child deepfork deepkid1 deepkid2 share sharekid maxfork arg_seq_child argtest argkid \
	cat cp filesynch filesynchkid

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

shell.o: shell.c
	$(CC) $(CFLAGS) -c shell.c
shell: shell.o start.o
	$(LD) $(LDFLAGS) start.o shell.o -o shell.coff
	../bin/coff2noff shell.coff shell

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult

fileio.o: fileio.c
	$(CC) $(CFLAGS) -c fileio.c
fileio: fileio.o start.o
	$(LD) $(LDFLAGS) start.o fileio.o -o fileio.coff
	../bin/coff2noff fileio.coff fileio 

fromcons.o: fromcons.c
	$(CC) $(CFLAGS) -c fromcons.c
fromcons: fromcons.o start.o
	$(LD) $(LDFLAGS) start.o fromcons.o -o fromcons.coff
	../bin/coff2noff fromcons.coff fromcons 

hellocons.o: hellocons.c
	$(CC) $(CFLAGS) -c hellocons.c
hellocons: hellocons.o start.o
	$(LD) $(LDFLAGS) start.o hellocons.o -o hellocons.coff
	../bin/coff2noff hellocons.coff hellocons 

hellofile.o: hellofile.c
	$(CC) $(CFLAGS) -c hellofile.c
hellofile: hellofile.o start.o
	$(LD) $(LDFLAGS) start.o hellofile.o -o hellofile.coff
	../bin/coff2noff hellofile.coff hellofile 

newfork.o: newfork.c
	$(CC) $(CFLAGS) -c newfork.c
newfork: newfork.o start.o
	$(LD) $(LDFLAGS) start.o newfork.o -o newfork.coff
	../bin/coff2noff newfork.coff newfork

parent_child.o: parent_child.c
	$(CC) $(CFLAGS) -c parent_child.c
parent_child: parent_child.o start.o
	$(LD) $(LDFLAGS) start.o parent_child.o -o parent_child.coff
	../bin/coff2noff parent_child.coff parent_child

kid.o: kid.c
	$(CC) $(CFLAGS) -c kid.c
kid: kid.o start.o
	$(LD) $(LDFLAGS) start.o kid.o -o kid.coff
	../bin/coff2noff kid.coff kid 

seq_child.o: seq_child.c
	$(CC) $(CFLAGS) -c seq_child.c
seq_child: seq_child.o start.o
	$(LD) $(LDFLAGS) start.o seq_child.o -o seq_child.coff
	../bin/coff2noff seq_child.coff seq_child 

deepfork.o: deepfork.c
	$(CC) $(CFLAGS) -c deepfork.c
deepfork: deepfork.o start.o
	$(LD) $(LDFLAGS) start.o deepfork.o -o deepfork.coff
	../bin/coff2noff deepfork.coff deepfork 

deepkid1.o: deepkid1.c
	$(CC) $(CFLAGS) -c deepkid1.c
deepkid1: deepkid1.o start.o
	$(LD) $(LDFLAGS) start.o deepkid1.o -o deepkid1.coff
	../bin/coff2noff deepkid1.coff deepkid1 

deepkid2.o: deepkid2.c
	$(CC) $(CFLAGS) -c deepkid2.c
deepkid2: deepkid2.o start.o
	$(LD) $(LDFLAGS) start.o deepkid2.o -o deepkid2.coff
	../bin/coff2noff deepkid2.coff deepkid2 

share.o: share.c
	$(CC) $(CFLAGS) -c share.c
share: share.o start.o
	$(LD) $(LDFLAGS) start.o share.o -o share.coff
	../bin/coff2noff share.coff share 

sharekid.o: sharekid.c
	$(CC) $(CFLAGS) -c sharekid.c
sharekid: sharekid.o start.o
	$(LD) $(LDFLAGS) start.o sharekid.o -o sharekid.coff
	../bin/coff2noff sharekid.coff sharekid 

maxfork.o: maxfork.c
	$(CC) $(CFLAGS) -c maxfork.c
maxfork: maxfork.o start.o
	$(LD) $(LDFLAGS) start.o maxfork.o -o maxfork.coff
	../bin/coff2noff maxfork.coff maxfork 

arg_seq_child.o: arg_seq_child.c
	$(CC) $(CFLAGS) -c arg_seq_child.c
arg_seq_child: arg_seq_child.o start.o
	$(LD) $(LDFLAGS) start.o arg_seq_child.o -o arg_seq_child.coff
	../bin/coff2noff arg_seq_child.coff arg_seq_child 

argtest.o: argtest.c
	$(CC) $(CFLAGS) -c argtest.c
argtest: argtest.o start.o
	$(LD) $(LDFLAGS) start.o argtest.o -o argtest.coff
	../bin/coff2noff argtest.coff argtest 

argkid.o: argkid.c
	$(CC) $(CFLAGS) -c argkid.c
argkid: argkid.o start.o
	$(LD) $(LDFLAGS) start.o argkid.o -o argkid.coff
	../bin/coff2noff argkid.coff argkid 

cat.o: cat.c
	$(CC) $(CFLAGS) -c cat.c
cat: cat.o start.o
	$(LD) $(LDFLAGS) start.o cat.o -o cat.coff
	../bin/coff2noff cat.coff cat 

cp.o: cp.c
	$(CC) $(CFLAGS) -c cp.c
cp: cp.o start.o
	$(LD) $(LDFLAGS) start.o cp.o -o cp.coff
	../bin/coff2noff cp.coff cp 

filesynch.o: filesynch.c
	$(CC) $(CFLAGS) -c filesynch.c
filesynch: filesynch.o start.o
	$(LD) $(LDFLAGS) start.o filesynch.o -o filesynch.coff
	../bin/coff2noff filesynch.coff filesynch 

filesynchkid.o: filesynchkid.c
	$(CC) $(CFLAGS) -c filesynchkid.c
filesynchkid: filesynchkid.o start.o
	$(LD) $(LDFLAGS) start.o filesynchkid.o -o filesynchkid.coff
	../bin/coff2noff filesynchkid.coff filesynchkid

