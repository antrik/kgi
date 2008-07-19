/*
 * Copyright 1997, 1998 by UCHIYAMA Yasushi
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of UCHIYAMA Yasushi not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  UCHIYAMA Yasushi makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * UCHIYAMA YASUSHI DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL UCHIYAMA YASUSHI BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */

#define _GNU_SOURCE

#include <stdlib.h>

#include <mach.h>
#include <device/device.h>
#include <mach/machine/mach_i386.h>
#include <hurd.h>

/**************************************************************************
 * Video Memory Mapping section                                            
 ***************************************************************************/
void * 
xf86MapVidMem(int ScreenNum,int Flags, unsigned long Base, unsigned long Size)
{
    mach_port_t device,iopl_dev;
    memory_object_t iopl_mem;
    kern_return_t err;
    vm_address_t addr=(vm_address_t)0;

    err = get_privileged_ports (NULL, &device);
    if( err )
    {
	errno = err;
	FatalError("xf86MapVidMem() can't get_privileged_ports. (%s)\n",strerror(errno));
    }
    err = device_open(device,D_READ|D_WRITE,"iopl",&iopl_dev);
    mach_port_deallocate (mach_task_self(), device);
    if( err )
    {
	errno = err;
	FatalError("xf86MapVidMem() can't device_open. (%s)\n",strerror(errno));
    }

    err = device_map(iopl_dev,VM_PROT_READ|VM_PROT_WRITE, Base , Size ,&iopl_mem,0);
    if( err )
    {
	errno = err;
	FatalError("xf86MapVidMem() can't device_map. (%s)\n",strerror(errno));
    }
    err = vm_map(mach_task_self(),
		 &addr,
		 Size,
		 0,     /* mask */
		 1,  /* anywhere */
		 iopl_mem,
		 (vm_offset_t)Base,
		 0, /* copy on write */
		 VM_PROT_READ|VM_PROT_WRITE,
		 VM_PROT_READ|VM_PROT_WRITE,
		 VM_INHERIT_SHARE);
    mach_port_deallocate(mach_task_self(),iopl_mem);
    if( err )
    {
	errno = err;
	FatalError("xf86MapVidMem() can't vm_map.(iopl_mem) (%s)\n",strerror(errno));
    }
    mach_port_deallocate(mach_task_self(),iopl_dev);
    if( err )
    {
	errno = err;
	FatalError("xf86MapVidMem() can't mach_port_deallocate.(iopl_dev) (%s)\n",strerror(errno));
    }
    return (void *)addr;
}

void 
xf86UnMapVidMem(int ScreenNum,void *Base,unsigned long Size)
{
    kern_return_t err = vm_deallocate(mach_task_self(), (int)Base, Size);
    if( err )
    {
	errno = err;
	ErrorF("xf86UnMapVidMem: can't dealloc framebuffer space (%s)\n",strerror(errno));
    }
    return;
}

int 
xf86LinearVidMem()
{
    return(1);
}

/**************************************************************************
 * I/O Permissions section                                                 
 ***************************************************************************/

/*
 * Due to conflicts with "compiler.h", don't rely on <sys/io.h> to declare
 * this.
 */
extern int ioperm(unsigned long __from, unsigned long __num, int __turn_on);

int
xf86EnableIO()
{
    if (ioperm(0, 0xffff, 1)) {
	FatalError("xf86EnableIO: ioperm() failed (%s)\n", strerror(errno));
	return 0;
    }
    ioperm(0x40,4,0); /* trap access to the timer chip */
    ioperm(0x60,4,0); /* trap access to the keyboard controller */
    return 1;
}
	
void
xf86DisableIO()
{
    ioperm(0,0xffff,0);
    return;
}

/**************************************************************************
 * Interrupt Handling section                                              
 **************************************************************************/
int 
xf86DisableInterrupts()
{
    return 1;
}
void 
xf86EnableInterrupts()
{
    return;
}

void
xf86MapReadSideEffects(int ScreenNum, int Flags, void *Base,
	unsigned long Size)
{
}

int
xf86CheckMTRR(int s)
{
	return 0;
}

