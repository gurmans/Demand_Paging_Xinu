# Demand_Paging_Xinu
## Lab Setup Guide
XINU is a small Unix-like operating system originally developed by Douglas Comer for instructional purposes at Purdue University. It is small enough so that we can understand it entirely within one semester. As part of lab assignment, we will re-implement or improve some apsects of XINU.
### Step 0: Installing QEMU

Following the instructions at the xv6 site to install QEMU.
I have tested this on Ubuntu 14.04, and it works. However, the QEMU from "apt-get install qemu" has problems. I have also tried Ubuntu 18.04, but it does not work well with the main problem that the XINU tarball fails to compile properly.

After you install QEMU, you need to change your PATH environment accordingly so that the later "make run" and "make debug" commands can find the binary.

If this is not enough for you to setup a local QEMU environment, I would suggest you just use the VCL environment. Post your questions to Moodle if you need help regarding local QEMU setup.

Step 1: Setting environment variables for lab assignments:

Get access to a customized VCL image -- XINU+QEMU (CSC501) -- through the VCL facility

### Step 2: Untar the XINU source files as follows:

Change to a working directory you would like to use for this project, preferable using /afs/unity.ncsu.edu/users/m/myid/this/is/my/project/dir
cd /afs/unity.ncsu.edu/users/m/myid/this/is/my/project/dir
Untar the XINU source by typing the following:
wget -U firefox https://people.engr.ncsu.edu/gjin2/Classes/501/Fall2019/assignments/PA0/csc501-lab0.tgz
tar xzvf csc501-lab0.tgz
In your working directory, you will now have a directory named csc501-lab0. The subdirectories under this directory contain source code, header files, etc, for XINU.

NOTE: the tar file name may be different from the above depending on the project you are working on. Please refer to the project handouts for the location of the tar file for the current project.

If you have a way to get rid of "-U firefox", let the instructor know.

### Step 3: Building XINU

To compile the XINU kernel which will be downloaded and run on the backend machines, run "make" in the compile directory as follows:
cd csc501-lab0/compile
make depend
make
This creates an OS image called 'xinu.elf'.
If you use the local QEMU environment, you may need to change two lines in the Makefile:

LD      =       /usr/bin/ld
to
LD      =       /usr/bin/gcc
and
	$(LD) -m elf_i386 -dn -Ttext 0x10000 -e start ${XOBJ} ${OBJ} ${LIB}/libxc.a \
to
	$(LD) -m32 -dn -Ttext 0x10000 -e start ${XOBJ} ${OBJ} ${LIB}/libxc.a \

### Step 4: Running and debugging XINU

The XINU image runs on the QEMU emulator machines. To boot up the image, type:

make run

XINU should start running and print a message "Hello World, Xinu lives."

Typing "Ctrl-a" then "c" will always bring you to "(qemu)" prompt. From there, you can quit by typing q.

To debug XINU, run the image in the debug mode by:

make debug

Then execute GDB in another ssh session:

gdb xinu.elf

In the (gdb) console, connect to the remote server by:

target remote localhost:1234

You can use many debugging features provided by GDB, e.g., adding a break point at the main function:

b main

To run to the next breakpoint, type:

c

The detailed document for GDB can be found here.
