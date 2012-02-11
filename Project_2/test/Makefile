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

# If you are running on SunOS 5.x machines, uncomment the next line
GCCDIR = /auto/home-scf-22/csci402/testgnu/decstation-ultrix/bin/
LDFLAGS = -T newscript -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR =
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)

CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld
CPP = $(GCCDIR)cpp

INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR)

all: halt matmult sort testfiles yield threadtest threadtest2 locktest locktest2 cvtest cvtest2 cvtest3 locktest3 movietheature movietheature1 movietheature2 movietheature3 threadtest breakExec breakForkTest scanTest execMatmult randomTest ExecTest2

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

yield.o: yield.c
	$(CC) $(CFLAGS) -c yield.c
yield: yield.o start.o
	$(LD) $(LDFLAGS) start.o yield.o -o yield.coff
	../bin/coff2noff yield.coff yield

movietheature.o: movietheature.c
	$(CC) $(CFLAGS) -c movietheature.c
movietheature: movietheature.o start.o
	$(LD) $(LDFLAGS) start.o movietheature.o -o movietheature.coff
	../bin/coff2noff movietheature.coff movietheature
	
movietheature1.o: movietheature1.c
	$(CC) $(CFLAGS) -c movietheature1.c
movietheature1: movietheature1.o start.o
	$(LD) $(LDFLAGS) start.o movietheature1.o -o movietheature1.coff
	../bin/coff2noff movietheature1.coff movietheature1
	
movietheature2.o: movietheature2.c
	$(CC) $(CFLAGS) -c movietheature2.c
movietheature2: movietheature2.o start.o
	$(LD) $(LDFLAGS) start.o movietheature2.o -o movietheature2.coff
	../bin/coff2noff movietheature2.coff movietheature2
	

movietheature3.o: movietheature3.c
	$(CC) $(CFLAGS) -c movietheature3.c
movietheature3: movietheature3.o start.o
	$(LD) $(LDFLAGS) start.o movietheature3.o -o movietheature3.coff
	../bin/coff2noff movietheature3.coff movietheature3
	
threadtest.o: threadtest.c
	$(CC) $(CFLAGS) -c threadtest.c
threadtest: threadtest.o start.o
	$(LD) $(LDFLAGS) start.o threadtest.o -o threadtest.coff
	../bin/coff2noff threadtest.coff threadtest

threadtest2.o: threadtest2.c
	$(CC) $(CFLAGS) -c threadtest2.c
threadtest2: threadtest2.o start.o
	$(LD) $(LDFLAGS) start.o threadtest2.o -o threadtest2.coff
	../bin/coff2noff threadtest2.coff threadtest2
	
locktest.o: locktest.c
	$(CC) $(CFLAGS) -c locktest.c
locktest: locktest.o start.o
	$(LD) $(LDFLAGS) start.o locktest.o -o locktest.coff
	../bin/coff2noff locktest.coff locktest

breakForkTest.o: breakForkTest.c
	$(CC) $(CFLAGS) -c breakForkTest.c
breakForkTest: breakForkTest.o start.o
	$(LD) $(LDFLAGS) start.o breakForkTest.o -o breakForkTest.coff
	../bin/coff2noff breakForkTest.coff breakForkTest

execMatmult.o: execMatmult.c
	$(CC) $(CFLAGS) -c execMatmult.c
execMatmult: execMatmult.o start.o
	$(LD) $(LDFLAGS) start.o execMatmult.o -o execMatmult.coff
	../bin/coff2noff execMatmult.coff execMatmult

scanTest.o: scanTest.c
	$(CC) $(CFLAGS) -c scanTest.c
scanTest: scanTest.o start.o
	$(LD) $(LDFLAGS) start.o scanTest.o -o scanTest.coff
	../bin/coff2noff scanTest.coff scanTest

randomTest.o: randomTest.c
	$(CC) $(CFLAGS) -c randomTest.c
randomTest: randomTest.o start.o
	$(LD) $(LDFLAGS) start.o randomTest.o -o randomTest.coff
	../bin/coff2noff randomTest.coff randomTest

breakExec.o: breakExec.c
	$(CC) $(CFLAGS) -c breakExec.c
breakExec: breakExec.o start.o
	$(LD) $(LDFLAGS) start.o breakExec.o -o breakExec.coff
	../bin/coff2noff breakExec.coff breakExec

locktest2.o: locktest2.c
	$(CC) $(CFLAGS) -c locktest2.c
locktest2: locktest2.o start.o
	$(LD) $(LDFLAGS) start.o locktest2.o -o locktest2.coff
	../bin/coff2noff locktest2.coff locktest2	

locktest3.o: locktest3.c
	$(CC) $(CFLAGS) -c locktest3.c
locktest3: locktest3.o start.o
	$(LD) $(LDFLAGS) start.o locktest3.o -o locktest3.coff
	../bin/coff2noff locktest3.coff locktest3		
	
cvtest.o: cvtest.c
	$(CC) $(CFLAGS) -c cvtest.c
cvtest: cvtest.o start.o
	$(LD) $(LDFLAGS) start.o cvtest.o -o cvtest.coff
	../bin/coff2noff cvtest.coff cvtest	
	
cvtest2.o: cvtest2.c
	$(CC) $(CFLAGS) -c cvtest2.c
cvtest2: cvtest2.o start.o
	$(LD) $(LDFLAGS) start.o cvtest2.o -o cvtest2.coff
	../bin/coff2noff cvtest2.coff cvtest2	

cvtest3.o: cvtest3.c
	$(CC) $(CFLAGS) -c cvtest3.c
cvtest3: cvtest3.o start.o
	$(LD) $(LDFLAGS) start.o cvtest3.o -o cvtest3.coff
	../bin/coff2noff cvtest3.coff cvtest3	


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

testfiles.o: testfiles.c
	$(CC) $(CFLAGS) -c testfiles.c
testfiles: testfiles.o start.o
	$(LD) $(LDFLAGS) start.o testfiles.o -o testfiles.coff
	../bin/coff2noff testfiles.coff testfiles

ExecTest2.o: ExecTest2.c
	$(CC) $(CFLAGS) -c ExecTest2.c
ExecTest2: ExecTest2.o start.o
	$(LD) $(LDFLAGS) start.o ExecTest2.o -o ExecTest2.coff
	../bin/coff2noff ExecTest2.coff ExecTest2
	
threadtest.o: threadtest.c
	$(CC) $(CFLAGS) -c threadtest.c
threadtest: threadtest.o start.o
	$(LD) $(LDFLAGS) start.o threadtest.o -o threadtest.coff
	../bin/coff2noff threadtest.coff threadtest
	
clean:
	rm -f *.o *.coff
