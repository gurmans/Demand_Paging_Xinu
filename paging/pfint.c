/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{

        int z =0;
        //kprintf("PD location: %d",((int)page_directory)/NBPG);
        for (;z<1024;z++)
        {
                if(page_directory[z].pd_pres == 1)
                {
        //              kprintf("PD[%d] value %d\n", z,page_directory[z].pd_base);
        //              kprintf("PageTables under it:\n");
                        pt_t *pt = page_directory[z].pd_base * NBPG;
                        int y=0;
                        for(;y<1024;y++)
                        {
                                if(pt[y].pt_pres)
                                {
        //                              kprintf("\tPT[%d]: %d\n", y, pt[y].pt_base);
                                }
                        }
                }
        }
  return OK;
}


