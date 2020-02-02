#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

//isBSBookedForPrivateHeapOrPageHolder(int i);

int get_bs(bsd_t bs_id, unsigned int npages) {

  /* requests a new mapping of npages with ID map_id */
//        kprintf("get_bs implemented!1\n");
	if(bs_id<0 || bs_id>=NBS || npages < 1 || npages > BACKING_STORE_UNIT_SIZE / NBPG)
                return SYSERR;
        if(isBSBookedForPrivateHeap(bs_id))
		return SYSERR;

    return BACKING_STORE_UNIT_SIZE / NBPG;

}

int isBSBookedForPrivateHeap(int i)
{
	return bsm_tab[i].bs_status == BSM_MAPPED && bsm_tab[i].bs_type == BS_VHEAP;
}
