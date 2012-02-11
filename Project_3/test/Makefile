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

all: halt matmult sort testfiles execMatmult DistLock DistCVTest1_1 DistCVTest1_2 DistCVTest2_1 DistCVTest2_2 DistCVTest2_3 DistCVTest3_1 DistCVTest3_2 DistCVTest3_3 DistCVTest3_4 DistCVTest3_5 DistCVTest4_1 DistCVTest4_2 DistCVTest4_3 DistCVTest5_1 DistCVTest5_2 DistCVTest5_3 DistLockTest6_1 DistLockTest6_2 DistLockTest6_3 DistCVTest7_1 DistCVTest7_2 DistCVTest7_3 DistMVTest8_1 DistMVTest8_2 DistMVTest8_3 DistCVTest9_1 DistCVTest9_2 execForkMatmult forkMatmult execSort forkSort execForkSort forkMatmult1 execSort1 forkSort1 execMatmult1

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

threadtest.o: threadtest.c
	$(CC) $(CFLAGS) -c threadtest.c
threadtest: threadtest.o start.o
	$(LD) $(LDFLAGS) start.o threadtest.o -o threadtest.coff
	../bin/coff2noff threadtest.coff threadtest

DistLock.o: DistLock.c
	$(CC) $(CFLAGS) -c DistLock.c
DistLock: DistLock.o start.o
	$(LD) $(LDFLAGS) start.o DistLock.o -o DistLock.coff
	../bin/coff2noff DistLock.coff DistLock

DistCVTest1_1.o: DistCVTest1_1.c
	$(CC) $(CFLAGS) -c DistCVTest1_1.c
DistCVTest1_1: DistCVTest1_1.o start.o
	$(LD) $(LDFLAGS) start.o DistCVTest1_1.o -o DistCVTest1_1.coff
	../bin/coff2noff DistCVTest1_1.coff DistCVTest1_1
	

DistCVTest1_2.o: DistCVTest1_2.c
	$(CC) $(CFLAGS) -c DistCVTest1_2.c
DistCVTest1_2: DistCVTest1_2.o start.o
	$(LD) $(LDFLAGS) start.o DistCVTest1_2.o -o DistCVTest1_2.coff
	../bin/coff2noff DistCVTest1_2.coff DistCVTest1_2
	
DistCVTest2_1.o: DistCVTest2_1.c
	$(CC) $(CFLAGS) -c DistCVTest2_1.c
DistCVTest2_1: DistCVTest2_1.o start.o
	$(LD) $(LDFLAGS) start.o DistCVTest2_1.o -o DistCVTest2_1.coff
	../bin/coff2noff DistCVTest2_1.coff DistCVTest2_1

DistCVTest2_2.o: DistCVTest2_2.c
	$(CC) $(CFLAGS) -c DistCVTest2_2.c
DistCVTest2_2: DistCVTest2_2.o start.o
	$(LD) $(LDFLAGS) start.o DistCVTest2_2.o -o DistCVTest2_2.coff
	../bin/coff2noff DistCVTest2_2.coff DistCVTest2_2

DistCVTest2_3.o: DistCVTest2_3.c
	$(CC) $(CFLAGS) -c DistCVTest2_3.c
DistCVTest2_3: DistCVTest2_3.o start.o
	$(LD) $(LDFLAGS) start.o DistCVTest2_3.o -o DistCVTest2_3.coff
	../bin/coff2noff DistCVTest2_3.coff DistCVTest2_3

DistCVTest3_1.o: DistCVTest3_1.c
	$(CC) $(CFLAGS) -c DistCVTest3_1.c
DistCVTest3_1: DistCVTest3_1.o start.o
	$(LD) $(LDFLAGS) start.o DistCVTest3_1.o -o DistCVTest3_1.coff
	../bin/coff2noff DistCVTest3_1.coff DistCVTest3_1

DistCVTest3_2.o: DistCVTest3_2.c
	$(CC) $(CFLAGS) -c DistCVTest3_2.c
DistCVTest3_2: DistCVTest3_2.o start.o
	$(LD) $(LDFLAGS) start.o DistCVTest3_2.o -o DistCVTest3_2.coff
	../bin/coff2noff DistCVTest3_2.coff DistCVTest3_2

DistCVTest3_3.o: DistCVTest3_3.c
	$(CC) $(CFLAGS) -c DistCVTest3_3.c
DistCVTest3_3: DistCVTest3_3.o start.o
	$(LD) $(LDFLAGS) start.o DistCVTest3_3.o -o DistCVTest3_3.coff
	../bin/coff2noff DistCVTest3_3.coff DistCVTest3_3

DistCVTest3_4.o: DistCVTest3_4.c
	$(CC) $(CFLAGS) -c DistCVTest3_4.c
DistCVTest3_4: DistCVTest3_4.o start.o
	$(LD) $(LDFLAGS) start.o DistCVTest3_4.o -o DistCVTest3_4.coff
	../bin/coff2noff DistCVTest3_4.coff DistCVTest3_4

DistCVTest3_5.o: DistCVTest3_5.c
	$(CC) $(CFLAGS) -c DistCVTest3_5.c
DistCVTest3_5: DistCVTest3_5.o start.o
	$(LD) $(LDFLAGS) start.o DistCVTest3_5.o -o DistCVTest3_5.coff
	../bin/coff2noff DistCVTest3_5.coff DistCVTest3_5

DistCVTest4_1.o: DistCVTest4_1.c
	$(CC) $(CFLAGS) -c DistCVTest4_1.c
DistCVTest4_1: DistCVTest4_1.o start.o
	$(LD) $(LDFLAGS) start.o DistCVTest4_1.o -o DistCVTest4_1.coff
	../bin/coff2noff DistCVTest4_1.coff DistCVTest4_1

DistCVTest4_2.o: DistCVTest4_2.c
	$(CC) $(CFLAGS) -c DistCVTest4_2.c
DistCVTest4_2: DistCVTest4_2.o start.o
	$(LD) $(LDFLAGS) start.o DistCVTest4_2.o -o DistCVTest4_2.coff
	../bin/coff2noff DistCVTest4_2.coff DistCVTest4_2

DistCVTest4_3.o: DistCVTest4_3.c
	$(CC) $(CFLAGS) -c DistCVTest4_3.c
DistCVTest4_3: DistCVTest4_3.o start.o
	$(LD) $(LDFLAGS) start.o DistCVTest4_3.o -o DistCVTest4_3.coff
	../bin/coff2noff DistCVTest4_3.coff DistCVTest4_3
	
DistCVTest5_1.o: DistCVTest5_1.c
	$(CC) $(CFLAGS) -c DistCVTest5_1.c
DistCVTest5_1: DistCVTest5_1.o start.o
	$(LD) $(LDFLAGS) start.o DistCVTest5_1.o -o DistCVTest5_1.coff
	../bin/coff2noff DistCVTest5_1.coff DistCVTest5_1

DistCVTest5_2.o: DistCVTest5_2.c
	$(CC) $(CFLAGS) -c DistCVTest5_2.c
DistCVTest5_2: DistCVTest5_2.o start.o
	$(LD) $(LDFLAGS) start.o DistCVTest5_2.o -o DistCVTest5_2.coff
	../bin/coff2noff DistCVTest5_2.coff DistCVTest5_2

DistCVTest5_3.o: DistCVTest5_3.c
	$(CC) $(CFLAGS) -c DistCVTest5_3.c
DistCVTest5_3: DistCVTest5_3.o start.o
	$(LD) $(LDFLAGS) start.o DistCVTest5_3.o -o DistCVTest5_3.coff
	../bin/coff2noff DistCVTest5_3.coff DistCVTest5_3

DistLockTest6_1.o: DistLockTest6_1.c
	$(CC) $(CFLAGS) -c DistLockTest6_1.c
DistLockTest6_1: DistLockTest6_1.o start.o
	$(LD) $(LDFLAGS) start.o DistLockTest6_1.o -o DistLockTest6_1.coff
	../bin/coff2noff DistLockTest6_1.coff DistLockTest6_1

DistLockTest6_2.o: DistLockTest6_2.c
	$(CC) $(CFLAGS) -c DistLockTest6_2.c
DistLockTest6_2: DistLockTest6_2.o start.o
	$(LD) $(LDFLAGS) start.o DistLockTest6_2.o -o DistLockTest6_2.coff
	../bin/coff2noff DistLockTest6_2.coff DistLockTest6_2

DistLockTest6_3.o: DistLockTest6_3.c
	$(CC) $(CFLAGS) -c DistLockTest6_3.c
DistLockTest6_3: DistLockTest6_3.o start.o
	$(LD) $(LDFLAGS) start.o DistLockTest6_3.o -o DistLockTest6_3.coff
	../bin/coff2noff DistLockTest6_3.coff DistLockTest6_3	

DistCVTest7_1.o: DistCVTest7_1.c
	$(CC) $(CFLAGS) -c DistCVTest7_1.c
DistCVTest7_1: DistCVTest7_1.o start.o
	$(LD) $(LDFLAGS) start.o DistCVTest7_1.o -o DistCVTest7_1.coff
	../bin/coff2noff DistCVTest7_1.coff DistCVTest7_1

DistCVTest7_2.o: DistCVTest7_2.c
	$(CC) $(CFLAGS) -c DistCVTest7_2.c
DistCVTest7_2: DistCVTest7_2.o start.o
	$(LD) $(LDFLAGS) start.o DistCVTest7_2.o -o DistCVTest7_2.coff
	../bin/coff2noff DistCVTest7_2.coff DistCVTest7_2

DistCVTest7_3.o: DistCVTest7_3.c
	$(CC) $(CFLAGS) -c DistCVTest7_3.c
DistCVTest7_3: DistCVTest7_3.o start.o
	$(LD) $(LDFLAGS) start.o DistCVTest7_3.o -o DistCVTest7_3.coff
	../bin/coff2noff DistCVTest7_3.coff DistCVTest7_3
	
DistMVTest8_1.o: DistMVTest8_1.c
	$(CC) $(CFLAGS) -c DistMVTest8_1.c
DistMVTest8_1: DistMVTest8_1.o start.o
	$(LD) $(LDFLAGS) start.o DistMVTest8_1.o -o DistMVTest8_1.coff
	../bin/coff2noff DistMVTest8_1.coff DistMVTest8_1

DistMVTest8_2.o: DistMVTest8_2.c
	$(CC) $(CFLAGS) -c DistMVTest8_2.c
DistMVTest8_2: DistMVTest8_2.o start.o
	$(LD) $(LDFLAGS) start.o DistMVTest8_2.o -o DistMVTest8_2.coff
	../bin/coff2noff DistMVTest8_2.coff DistMVTest8_2

DistMVTest8_3.o: DistMVTest8_3.c
	$(CC) $(CFLAGS) -c DistMVTest8_3.c
DistMVTest8_3: DistMVTest8_3.o start.o
	$(LD) $(LDFLAGS) start.o DistMVTest8_3.o -o DistMVTest8_3.coff
	../bin/coff2noff DistMVTest8_3.coff DistMVTest8_3

DistCVTest9_1.o: DistCVTest9_1.c
	$(CC) $(CFLAGS) -c DistCVTest9_1.c
DistCVTest9_1: DistCVTest9_1.o start.o
	$(LD) $(LDFLAGS) start.o DistCVTest9_1.o -o DistCVTest9_1.coff
	../bin/coff2noff DistCVTest9_1.coff DistCVTest9_1	

DistCVTest9_2.o: DistCVTest9_2.c
	$(CC) $(CFLAGS) -c DistCVTest9_2.c
DistCVTest9_2: DistCVTest9_2.o start.o
	$(LD) $(LDFLAGS) start.o DistCVTest9_2.o -o DistCVTest9_2.coff
	../bin/coff2noff DistCVTest9_2.coff DistCVTest9_2	
	
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

execMatmult1.o: execMatmult1.c
	$(CC) $(CFLAGS) -c execMatmult1.c
execMatmult1: execMatmult1.o start.o
	$(LD) $(LDFLAGS) start.o execMatmult1.o -o execMatmult1.coff
	../bin/coff2noff execMatmult1.coff execMatmult1

execForkMatmult.o: execForkMatmult.c
	$(CC) $(CFLAGS) -c execForkMatmult.c
execForkMatmult: execForkMatmult.o start.o
	$(LD) $(LDFLAGS) start.o execForkMatmult.o -o execForkMatmult.coff
	../bin/coff2noff execForkMatmult.coff execForkMatmult

forkMatmult.o: forkMatmult.c
	$(CC) $(CFLAGS) -c forkMatmult.c
forkMatmult: forkMatmult.o start.o
	$(LD) $(LDFLAGS) start.o forkMatmult.o -o forkMatmult.coff
	../bin/coff2noff forkMatmult.coff forkMatmult

forkMatmult1.o: forkMatmult1.c
	$(CC) $(CFLAGS) -c forkMatmult1.c
forkMatmult1: forkMatmult1.o start.o
	$(LD) $(LDFLAGS) start.o forkMatmult1.o -o forkMatmult1.coff
	../bin/coff2noff forkMatmult1.coff forkMatmult1

	
execForkSort.o: execForkSort.c
	$(CC) $(CFLAGS) -c execForkSort.c
execForkSort: execForkSort.o start.o
	$(LD) $(LDFLAGS) start.o execForkSort.o -o execForkSort.coff
	../bin/coff2noff execForkSort.coff execForkSort	
	
forkSort.o: forkSort.c
	$(CC) $(CFLAGS) -c forkSort.c
forkSort: forkSort.o start.o
	$(LD) $(LDFLAGS) start.o forkSort.o -o forkSort.coff
	../bin/coff2noff forkSort.coff forkSort
	
execSort.o: execSort.c
	$(CC) $(CFLAGS) -c execSort.c
execSort: execSort.o start.o
	$(LD) $(LDFLAGS) start.o execSort.o -o execSort.coff
	../bin/coff2noff execSort.coff execSort	

forkSort1.o: forkSort1.c
	$(CC) $(CFLAGS) -c forkSort1.c
forkSort1: forkSort1.o start.o
	$(LD) $(LDFLAGS) start.o forkSort1.o -o forkSort1.coff
	../bin/coff2noff forkSort1.coff forkSort1
	
execSort1.o: execSort1.c
	$(CC) $(CFLAGS) -c execSort1.c
execSort1: execSort1.o start.o
	$(LD) $(LDFLAGS) start.o execSort1.o -o execSort1.coff
	../bin/coff2noff execSort1.coff execSort1
	
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
	

clean:
	rm -f *.o *.coff
