/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
        frmhead = frmtail = -1;
	fr_map_t *frmap = &frm_tab;
        int i=0;
        for(;i<NFRAMES;i++)
        {
               frmap->fr_status = FRM_UNMAPPED;
		frmap->nextfrmidx = -1;
		frmap->prevfrmidx = -1;
               frmap++;
         }
//	kprintf("To be implemented!\n");
  return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
        fr_map_t *frmap = &frm_tab;
        int i=0;
	//if(frmhead == -1 || frmtail == -1)	
        for(;i<NFRAMES;i++)
        {
                if(frmap->fr_status == FRM_UNMAPPED)
                {
                        *avail = i;
//kprintf("get_frm 1 returned %d!\n",i);
                        return i;
                }
                frmap++;
        }
	i = -1;
	i = recompute_age_pf_acc();
	if(i >=0 && evict_frame(i) != SYSERR){
		*avail = i;
//kprintf("get_frm 2 returned %d!\n",i);
		return i;
	}
	//no empty frame left, evict one page and return its address
	
  kprintf("Error in Page replacement!\n");
  return SYSERR;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{

        if(i<0 || i>=NFRAMES)
                return SYSERR;
	int vpno = frm_tab[i].fr_vpno;
        pd_t *page_directory = read_cr3();
	virt_addr_t faultedVaddr;
        faultedVaddr.pt_offset = (vpno & 1023);
        faultedVaddr.pd_offset = (vpno >> 10);
	pt_t *page_table = page_directory[faultedVaddr.pd_offset].pd_base * NBPG;

        if(frm_tab[i].fr_type == FR_PAGE && page_table[faultedVaddr.pt_offset].pt_dirty == 1)
        {
               if(write_bs((i + FRAME0)*NBPG, frm_tab[i].bs_id, frm_tab[i].bs_page) == SYSERR) {kprintf("write_bs failed\n");return SYSERR;}
	//	char *addr = BACKING_STORE_BASE + frm_tab[i].bs_id*BACKING_STORE_UNIT_SIZE + frm_tab[i].bs_page;
	//	char *addr2 = (i + FRAME0)*NBPG;
	//	kprintf("src: %c written as %c at :%d:%d\n",*addr2, *addr, frm_tab[i].bs_id,frm_tab[i].bs_page);
        }
	if(faultedVaddr.pd_offset >= 4 )
		page_table[faultedVaddr.pt_offset].pt_pres = 0;
        frm_tab[i].fr_status = FRM_UNMAPPED;
	frm_tab[i].fr_pid = -1;
	frm_tab[i].fr_vpno = -1;
	frm_tab[i].fr_refcnt = -1;
	frm_tab[i].fr_type = -1;
	frm_tab[i].fr_dirty = -1;
	frm_tab[i].bs_id = -1;
	frm_tab[i].bs_page = -1;
//  kprintf("free frame Implemented!\n");
  return OK;
}

SYSCALL free_all_frames(int pid)
{
	int i = 0;
        write_cr3(FRAME0*NBPG);
	for(;i<NFRAMES;i++)
	{
		if(frm_tab[i].fr_pid == pid)
		{
			if(frm_tab[i].fr_type == FR_PAGE)
			{
				delete_from_queue(i);
			}
			free_frm(i);
		}
	}

}
SYSCALL frame_map(int pid, int frame_number, int vpno, int type, int bs_id, int bs_page, int pt_addr, int pt_idx)
{
        frm_tab[frame_number].fr_status = FRM_MAPPED;
        frm_tab[frame_number].fr_pid = pid;
        frm_tab[frame_number].fr_vpno = vpno;
        frm_tab[frame_number].fr_type = type;
        frm_tab[frame_number].bs_id = bs_id;
        frm_tab[frame_number].bs_page = bs_page;
        frm_tab[frame_number].pt_addr = pt_addr;
        frm_tab[frame_number].pt_idx = pt_idx;
	if(type != FR_PAGE)
		return OK;//not adding PT's and PD's into the queue
	if(add_to_queue(frame_number) == SYSERR) 
		return SYSERR; 
	return OK;
}

SYSCALL add_to_queue(int frame_number)
{
//kprintf("add to queue 1:%d:%d\n", frmhead,frmtail);

        if(frmhead == -1 || frmtail == -1)
        {
                frmhead = frmtail = frame_number;
                frm_tab[frame_number].nextfrmidx = -1;
                frm_tab[frame_number].prevfrmidx = -1;
        }
	else
	{
		frm_tab[frmtail].nextfrmidx = frame_number;
		frm_tab[frame_number].prevfrmidx = frmtail;
		frm_tab[frame_number].nextfrmidx = -1;
                frmtail = frame_number;
	}
//kprintf("add to queue 2:%d:%d\n", frmhead,frmtail);

}

SYSCALL delete_from_queue_head()
{
	return delete_from_queue(frmhead);
}

SYSCALL delete_from_queue(int frame_number)
{
	int nextidx = frm_tab[frame_number].nextfrmidx;
        int previdx = frm_tab[frame_number].prevfrmidx;
//kprintf("delete from queue 1:%d:%d\n", frmhead,frmtail);

	if(frame_number == frmhead){
		frmhead = nextidx;
	        frm_tab[nextidx].prevfrmidx = previdx;
	}else if (frame_number == frmtail){
		frmtail = previdx;
		frm_tab[previdx].nextfrmidx = nextidx;
	} else {
        	frm_tab[nextidx].prevfrmidx = previdx;
	        frm_tab[previdx].nextfrmidx = nextidx;
	}
//kprintf("delete from queue 2:%d:%d\n", frmhead,frmtail);
	return OK;
}

SYSCALL recompute_age_pf_acc(){
        int i = frmhead;
        if(grpolicy() == SC)
        {
                while(i>=0)
                {  
                        int pt_idx = frm_tab[i].pt_idx;
                        pt_t *pt = frm_tab[i].pt_addr;
                        if(pt[pt_idx].pt_acc == 0)
                                return i;
                        pt[pt_idx].pt_acc = 0;
                        i = frm_tab[i].nextfrmidx;
                }
                i = frmhead;
                while(i>=0)
                {  
                        int pt_idx = frm_tab[i].pt_idx;
                        pt_t *pt = frm_tab[i].pt_addr;
                        if(pt[pt_idx].pt_acc == 0)
                                return i;
                        i = frm_tab[i].nextfrmidx;
                }
        } else if(grpolicy() == AGING)
        {
                while(i>=0)
                {
                        frm_tab[i].fr_age = frm_tab[i].fr_age >> 1;
                        int pt_idx = frm_tab[i].pt_idx;
                        pt_t *pt = frm_tab[i].pt_addr;
                        if(pt[pt_idx].pt_acc == 1)
                                frm_tab[i].fr_age += 128;
                        if(frm_tab[i].fr_age > 255)
                                frm_tab[i].fr_age = 255;
                        pt[pt_idx].pt_acc = 0;
                        i = frm_tab[i].nextfrmidx;
                }
                i = frmhead;
                int minage = 255, retval = i;
                while(i>=0)
                {
                        if(minage > frm_tab[i].fr_age)
                        {
                                minage = frm_tab[i].fr_age;
                                retval = i;
                        }
                        i = frm_tab[i].nextfrmidx;
                }
                return retval;
        }
}

SYSCALL evict_frame(int frame_number)
{
	int src = FRAME0 + frame_number;
//	free_frame(frame_number);
	if(free_frm(frame_number) == SYSERR)
		return SYSERR;
	return delete_from_queue(frame_number);
}
