/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
	virt_addr_t virtualAddress;
	virtualAddress.pt_offset = virtpage & 1111111111;
	virtualAddress.pd_offset = virtpage >> 10;

	if(source<0 || source>= NBS)
		return SYSERR;
	return bsm_map(currpid,virtpage,source,npages,BS_MMAP);	//xmmap is shared, so pid = -1, vpno = -1

//	pd_t *pd;
//	pt_t *pt;// = virtualAddress.pt_offset;
////	if(&proctab[currpid].PBDR) no need as process's pdbr is set during process creation
//	pd = &proctab[currpid].pdbr;
		
//	if(pd[virtualAddress.pd_offset].pd_pres)
//		pt = pd[virtualAddress.pd_offset].pd_base;
//	else {
//		pd[virtualAddress.pd_offset].pd_pres = 1;
//		pd[virtualAddress.pd_offset].pd_base = pt;
//	}
//
//	kprintf("xmmap - to be implemented!\n");

}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{
        char *bs_ids = &proctab[currpid].bs_ids[0];
	int *vpnos = &proctab[currpid].vpnos[0];
	int source,i=0;
        for(;i<NBS;i++)
        {
		if(*vpnos == virtpage)
		{
			*vpnos = 0;
			source = bs_ids[i];
			bs_ids[i] = -1;
			break;
		}
		vpnos++;
	}
	if(source == 0)
	{
		kprintf("Virtual Page not found");
		return SYSERR;
	}
        bsm_tab[source].bs_refcnt--;

//  kprintf("xmunmap implemented!");
  return 1;//SYSERR;
}
