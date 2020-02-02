/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>


/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
//TBD
//	&bsm_tab = BACKING_STORE_BASE;//5444547;
        bs_map_t *bsmap = &bsm_tab;

	int i=0;
	for(;i<NBS;i++)
	{
		bsmap->bs_status = BSM_UNMAPPED;
		bsmap++;
	}
//bs_map_t *bsm_tab = &bsm_tab;
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
        bs_map_t *bsmap = &bsm_tab;
        int i=0;
        for(;i<NBS;i++)
        {
              	if(bsmap->bs_status == BSM_UNMAPPED)
		{
			avail = &bsmap;
			return 1;
		}
		bsmap++;
        }
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
	if(i<0 || i>=NBS)
		return SYSERR;
	bsm_tab[i].bs_status = BSM_UNMAPPED;
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
/*
 *	get physical address from vaddr using page tables, (should lie in the BS range), then translate the phy address into BS_id(store) and store-page-number(pageth)
 */
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab
 * WIll create a mapping in all types of BS 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages, int type)
{
	if(type < 0)
		return SYSERR;
	if(source < 0)	//will be called internally from vcreate with -1 value
	{
        	bs_map_t *bsmap = &bsm_tab;
		int i = 0;
        	for(;i<NBS;i++)
	        {
        	        if(bsmap->bs_status == BSM_UNMAPPED)
                	{
				source = i;
				break;
                	}
                	bsmap++;
        	}
	}
        if(source == -1)
        {
                 kprintf("No more Backing Stores available");
                 return SYSERR;
        }

	bsm_tab[source].bs_status = BSM_MAPPED;
        bsm_tab[source].bs_pid = pid;
        bsm_tab[source].bs_vpno = vpno;
        bsm_tab[source].bs_npages = npages;
        bsm_tab[source].bs_type = type;
	if(type == BS_VHEAP)
	{
		bsm_tab[source].bs_refcnt = 1;
		proctab[pid].store = source;
		proctab[pid].vhpnpages = npages;
//yet to set vhpno, think about the original getmem execution for this
		proctab[pid].vhpno = vpno >> 12;
      		struct  mblock *mptr;
        	(*proctab[pid].vmemlist).mnext = mptr = (struct mblock *) roundmb(BACKING_STORE_BASE + (BACKING_STORE_UNIT_SIZE * source));
        	mptr->mnext = mptr + (npages * NBPG);
        	mptr->mlen = (int) npages * NBPG;

		mptr = (int) mptr->mnext + (npages * NBPG);
		mptr->mnext = 0;
		mptr->mlen = 0;
	}
	else if(type == BS_MMAP)
	{
	        bsm_tab[source].bs_pid = -1;
	        bsm_tab[source].bs_refcnt++;
		char *bs_ids = &proctab[pid].bs_ids[0];
                int *vpnos = &proctab[pid].vpnos[0];
		bs_ids[source] = source;
		vpnos[source] = vpno;
	}
        //kprintf("BS mapped %d proctab vpno: %d, bsm pid: %d, bsm refcnt: %d, mappes: %d\n", i, proctab[pid].vpnos[i], bsm_tab[i].bs_pid, bsm_tab[i].bs_refcnt, bsm_tab[i].bs_status);
	return source;
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)		//Only for Vheap
{
//        if(vpno<0 || vpno>= BACKING_STORE_UNIT_SIZE / NBPG) //256
//                return SYSERR;
        int i=0;
        for(;i<NBS;i++)
        {
                if(proctab[pid].vpnos[i] > 0 || bsm_tab[i].bs_pid == pid)
		{
			bsm_tab[i].bs_refcnt--;
		
			if(bsm_tab[i].bs_refcnt <= 0)
                	{
				free_bsm(i);
			}
		}
                //kprintf("BS Deleted %d proctab vpno: %d, bsm pid: %d, bsm refcnt: %d, mappes: %d\n", i, proctab[pid].vpnos[i], bsm_tab[i].bs_pid, bsm_tab[i].bs_refcnt, bsm_tab[i].bs_status);
	}
	return OK;
}


