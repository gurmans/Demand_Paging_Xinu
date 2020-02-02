/* vgetmem.c - vgetmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 * vgetmem  --  allocate virtual heap storage, returning lowest WORD address
 *------------------------------------------------------------------------
 */
WORD	*vgetmem(nbytes)
	unsigned nbytes;
{
	STATWORD ps;    
	struct	mblock	*p, *q, *leftover;

	disable(ps);
	if (nbytes==0 || nbytes/NBPG > proctab[currpid].vhpnpages || (*proctab[currpid].vmemlist).mnext == (struct mblock *) NULL) {
		//kprintf("1");
		restore(ps);
		return( (WORD *)SYSERR);
	}
	nbytes = (unsigned int) roundmb(nbytes);
	for (q= proctab[currpid].vmemlist, p = (*proctab[currpid].vmemlist).mnext;
	     p != (struct mblock *) NULL ;
	     q=p,p=p->mnext)
		if ( p->mlen == nbytes) {
			//kprintf("2");
			q->mnext = p->mnext;
			restore(ps);
			return( (WORD *)calculateVirtualAddress((int)p));
//proctab[currpid].vhpno + (p - (BACKING_STORE_BASE + (BACKING_STORE_UNIT_SIZE * proctab[currpid].store)))

		} else if ( p->mlen > nbytes ) {
                        //kprintf("3");
			leftover = (struct mblock *)( (unsigned)p + nbytes );
			q->mnext = leftover;
			leftover->mnext = p->mnext;
			leftover->mlen = p->mlen - nbytes;
			restore(ps);
			return( (WORD *)calculateVirtualAddress((int)p) );
		}
        //kprintf("4");
	restore(ps);
	return( (WORD *)SYSERR );

}

int calculateVirtualAddress(int p)
{
        //kprintf("Heap translation of address: %d", p);
	int a = BACKING_STORE_BASE + (BACKING_STORE_UNIT_SIZE * proctab[currpid].store);
	int rval = proctab[currpid].vhpno + (p - (BACKING_STORE_BASE + (BACKING_STORE_UNIT_SIZE * proctab[currpid].store)));
        //kprintf("Heap address after translation: %d", rval);
	return rval;
}
