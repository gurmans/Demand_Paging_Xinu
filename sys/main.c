#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <paging.h>


#define PROC1_VADDR	0x40000000
#define PROC1_VPNO      0x40000
#define PROC2_VADDR     0x80000000
#define PROC2_VPNO      0x80000
#define TEST1_BS	1

void proc1_test1(char *msg, int lck) {
	char *addr;
	int i;

	get_bs(TEST1_BS, 250);

	if (xmmap(PROC1_VPNO, TEST1_BS, 250) == SYSERR) {
		kprintf("xmmap call failed\n");
		sleep(3);
		return;
	}
	addr = (char*) PROC1_VADDR;
	for (i = 0; i < 25; i++) {
		*(addr + i * NBPG) = 'A' + i;
	//	kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
	}
	sleep(2);

	for (i = 0; i < 25; i++) {
		kprintf("0x%08x: %c\n",addr + i * NBPG, *(addr + i * NBPG));
	}

	xmunmap(PROC1_VPNO);
	release_bs(TEST1_BS);
	return;
}

void proc1_test2(char *msg, int lck) {
	int *x;

	kprintf("ready to allocate heap space\n");
	x = vgetmem(1024);
        kprintf("heap allocated at %x\n", x);

        *x = 100;
        *(x + 1) = 200;
	vfreemem(x,1024);
	kprintf("heap variable: %d %d\n", *x, *(x + 1));
        kprintf("ready to allocate heap space\n");
        x = vgetmem(1024);
        kprintf("heap allocated at %x\n", x);
        kprintf("heap variable: %d %d\n", *x, *(x + 1));
	vfreemem(x,100);	
}

void proc1_test3(char *msg, int lck) {
	char *addr;
	int i;

	addr = (char*) 0x0;

	for (i = 1024; i < 2048; i++) {
	//	*(addr + i * NBPG) = 'B';
	}

	for (i = 1024; i < 2048; i++) {
		kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
	}

	return;
}

int main() {
	int pid1;
	int pid2;

        char *addr;
        int i;

        get_bs(TEST1_BS, 250);

        if (xmmap(PROC1_VPNO, TEST1_BS, 250) == SYSERR) {
                kprintf("xmmap call failed\n");
                sleep(3);
                return;
        }
        addr = (char*) PROC1_VADDR;
        for (i = 0; i < 25; i++) {
                *(addr + i * NBPG) = 'A' + i;
        }
        sleep(2);

        for (i = 0; i < 25; i++) {
                kprintf("0x%08x: %c\n",addr + i * NBPG, *(addr + i * NBPG));
        }


	kprintf("\n1: shared memory \n");
	pid1 = create(proc1_test1, 2000, 20, "proc1_test1", 0, NULL);
	resume(pid1);
	sleep(10);
        xmunmap(PROC1_VPNO);
	release_bs(TEST1_BS);
	kprintf("\n2: vgetmem/vfreemem\n");
	pid1 = vcreate(proc1_test2, 2000, 100, 20, "proc1_test2", 0, NULL);
	kprintf("pid %d has private heap\n", pid1);
	kill(pid1);
	//	resume(pid1);
	sleep(3);

	kprintf("\n3: Frame test:\n");
	pid1 = vcreate(proc1_test3, 2000, 54, 20, "proc1_test3", 0, NULL);
        kprintf("\n3: Frame test:\n");
	resume(pid1);
	sleep(3);


	kprintf("dmp");
	shutdown();
}
