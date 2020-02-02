#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {

  /* release the backing store with ID bs_id */
	if(bsm_tab[bs_id].bs_refcnt > 0)
		kprintf("BS cannot be released as it is being used by other processes!\n");
	else
	{
		free_bsm(bs_id);
		//kprintf("BSM released\n");
	}
   return OK;

}

