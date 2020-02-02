/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 *  vfreemem  --  free a virtual memory block, returning it to vmemlist
 *------------------------------------------------------------------------
 */
SYSCALL	vfreemem(block, size)
	struct	mblock	*block;
	unsigned size;
{
	STATWORD ps;    
	struct	mblock	*p, *q;
	unsigned top;
	int source = proctab[currpid].store;
	block = (int)block - proctab[currpid].vhpno + (BACKING_STORE_BASE + (BACKING_STORE_UNIT_SIZE * proctab[currpid].store));

	if (size==0 || (unsigned)block>(unsigned)roundmb((BACKING_STORE_BASE) + (BACKING_STORE_UNIT_SIZE * source -1))
	    || ((unsigned)block)<((unsigned) roundmb(BACKING_STORE_BASE + (BACKING_STORE_UNIT_SIZE * source))))
	{
		kprintf("Error111111111:%d,:%d\n", block, (BACKING_STORE_BASE/NBPG) + (BACKING_STORE_UNIT_SIZE/NBPG * source)+(BACKING_STORE_UNIT_SIZE/NBPG) -1);
		return(SYSERR);
	}
	size = (unsigned)roundmb(size);
	disable(ps);
	for( q= proctab[currpid].vmemlist, p = (*proctab[currpid].vmemlist).mnext;
	     p != (struct mblock *) NULL && p < block ;
	     q=p,p=p->mnext )
		;
	if (((top=q->mlen+(unsigned)q)>(unsigned)block && q!= &memlist) ||
	    (p!=NULL && (size+(unsigned)block) > (unsigned)p )) {
		restore(ps);
	{kprintf("Error222222222\n");	return(SYSERR);}
	}
	if ( q!= proctab[currpid].vmemlist && top == (unsigned)block )
			q->mlen += size;
	else {
		block->mlen = size;
		block->mnext = p;
		q->mnext = block;
		q = block;
	}
	if ( (unsigned)( q->mlen + (unsigned)q ) == (unsigned)p) {
		q->mlen += p->mlen;
		q->mnext = p->mnext;
	}
	restore(ps);
	return(OK);
}
