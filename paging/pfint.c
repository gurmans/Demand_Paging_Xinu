/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
* Occuring becaue:	1.page directory entry had present == 0
* 		or	2.page table entry had present ==0
*
* 1. if page dir present == 0
* 	create a new page table at a new frame.
* 	
* 2. if page table present == 0
* 	copy the physical frame from BS to a new frame.
*
* 	get BS page from xmmap
*
 */
SYSCALL pfint()
{
	virt_addr_t faultedVaddr;// = read_cr2();
        // Disable interrupts
	STATWORD ps;
        disable(ps);
        int vpno = read_cr2() >> 12;
        faultedVaddr.pt_offset = (vpno & 1023);
        faultedVaddr.pd_offset = (vpno >> 10);
	// Disable interrupts
	disable(ps);


//TODO: Check that a is a legal address (i.e. that it has been mapped in pd). If it is not, print an error message and kill the process
//i.e., check if a is within 4GB ?

//finding the real page and copying it to the location freePhysicalFrameNumber
        struct pentry *process = &proctab[currpid];
        int i=0,bs_id = -1,bs_page_number = -1;

        for(;i<NBS;i++)
        {
                if((bsm_tab[i].bs_status == BSM_MAPPED) && (bsm_tab[i].bs_type == BS_MMAP) &&
                        ((vpno - process->vpnos[i]) >= 0) && ((vpno - process->vpnos[i]) < bsm_tab[i].bs_npages))
                {
                        bs_id = i;
                        bs_page_number = vpno - process->vpnos[i];
			//kprintf("BSID: %d, Page#: %d\n", bs_id,bs_page_number);
                        break;
                }
                else if((bsm_tab[i].bs_status == BSM_MAPPED) && (bsm_tab[i].bs_type == BS_VHEAP) &&
                        (bsm_tab[i].bs_pid == currpid) && ((vpno - process->vhpno) >= 0) && ((vpno - process->vhpno) < process->vhpnpages))
                {
			
                        bs_id = i;
                        bs_page_number = vpno - process->vhpno;
                        break;
                }
        }

	if(bs_id == -1)
	{
		
		restore(ps);
		kprintf("Killing process %d, VADDR: %d, Expected Base: %d\n", currpid, read_cr2() , &proctab[currpid].vpnos[2]);
		kill(currpid);
		return SYSERR;
	}

	pd_t *page_directory = read_cr3();
	int newPTFrameNumber,newPageFrameNumber;
//	kprintf("\n%lx",&page_directory[faultedVaddr.pd_offset]);
//	shutdown();
	if(page_directory[faultedVaddr.pd_offset].pd_pres == 0)
	{
		if(get_frm(&newPTFrameNumber) == SYSERR)
		{	 restore(ps);
                	kprintf("Killing process 2");
                	kill(currpid);
			return SYSERR;
		}
		page_directory[faultedVaddr.pd_offset].pd_base = FRAME0 + newPTFrameNumber;
		page_directory[faultedVaddr.pd_offset].pd_pres = 1;
		frame_map(currpid, newPTFrameNumber, vpno, FR_TBL, -1, -1, -1, -1);
	}
	pt_t *page_table = page_directory[faultedVaddr.pd_offset].pd_base * NBPG;
//	kprintf("\nPT: %lx",page_table);
//	shutdown();
        if(get_frm(&newPageFrameNumber) == SYSERR)
        {         restore(ps);
                kprintf("Killing process 3");
                kill(currpid);
		return SYSERR;
	}
	

	int dst;
	dst = (newPageFrameNumber + FRAME0);
	//if(bs_id == -1 || (read_bs((char*)(dst * NBPG), bs_id, bs_page_number) == SYSERR))
	//if( bs_id == -1 || read_bs((char*)(dst ), bs_id, bs_page_number) == SYSERR )

	if(read_bs((char *)(dst*NBPG), bs_id, bs_page_number) == SYSERR )
	{
		restore(ps);
                kprintf("Killing process 4");
                kill(currpid);
		return SYSERR;
	}

        page_table[faultedVaddr.pt_offset].pt_base = FRAME0 + newPageFrameNumber;
        page_table[faultedVaddr.pt_offset].pt_pres = 1;
	page_table[faultedVaddr.pt_offset].pt_acc = 0;
	page_table[faultedVaddr.pt_offset].pt_dirty = 0;
	frame_map(currpid, newPageFrameNumber, vpno, FR_PAGE, bs_id, bs_page_number, (int)page_table, faultedVaddr.pt_offset);
  //kprintf("Page fault To be implemented!\n");
	write_cr3(page_directory);//invalidating TLB's
//kprintf("Page fault To be implemented:%d,:%d,:%d, :%s,:%d!\n",read_cr2(),faultedVaddr.pt_offset,faultedVaddr.pd_offset, proctab[currpid].pname, proctab[currpid].pstate);
  restore(ps);
  return OK;
}


