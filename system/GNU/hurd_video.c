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

#include <error.h>
#include <stdlib.h>

#include <mach.h>
#include <mach/mach4.h>
#include <device/device.h>
#include <mach/machine/mach_i386.h>
#include <hurd.h>

#include "hurd_video.h"

/* Get a memory object for the whole physical address space. */
static memory_object_t get_physmem_object(void)
{
	mach_port_t master_device_port;
	mach_port_t iopl_dev;
	memory_object_t obj;
	kern_return_t err;

	/* first open master device port */
	err = get_privileged_ports(NULL, &master_device_port);
	if (err)
		error(2, err, "get_physmem_object() can't get master device port");

	/* now use it to open iopl device */
	err = device_open(master_device_port, VM_PROT_READ | VM_PROT_WRITE, "iopl", &iopl_dev);
	if (err)
		error(2, err, "get_physmem_object() can't open iopl device");

	mach_port_deallocate(mach_task_self(), master_device_port);

	/* now use that to get a memory object for the physical address space */
	err = device_map(iopl_dev, D_READ|D_WRITE, 0, ~0, &obj, 0);
	if (err)
		error(2, err, "get_physmem_object() can't obtain memory object");

	mach_port_deallocate(mach_task_self(), iopl_dev);

	return obj;
}

/*
 * Get a memory object for the given region of physical address space.
 *
 * As the kernel device always gives us a memory object for r/w access to the
 * whole address space, we need to use a proxy memory object to limit the
 * region and permissions.
 */
memory_object_t get_fb_object(vm_address_t addr, vm_size_t size, vm_prot_t prot)
{
	memory_object_t mem_obj, proxy_obj;
	kern_return_t err;

	mem_obj = get_physmem_object();

	{
		mach_port_t object[] = { mem_obj };
		vm_offset_t offset[] = { addr };
		vm_offset_t start[] = { 0 };
		vm_offset_t len[] = { size };

		err = memory_object_create_proxy(mach_task_self(), prot, object, 1, offset, 1, start, 1, len, 1, &proxy_obj);
		if (err)
			error(2, err, "get_fb_object() can't obtain proxy memory object");
	}

	return proxy_obj;
}


/**************************************************************************
 * Video Memory Mapping section                                            
 ***************************************************************************/
void * 
xf86MapVidMem(int ScreenNum,int Flags, unsigned long Base, unsigned long Size)
{
    memory_object_t iopl_mem;
    kern_return_t err;
    vm_address_t addr=(vm_address_t)0;

    iopl_mem = get_physmem_object();

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
	error(1, err, "xf86MapVidMem() can't vm_map");
    }
    return (void *)addr;
}

void 
xf86UnMapVidMem(int ScreenNum,void *Base,unsigned long Size)
{
    kern_return_t err = vm_deallocate(mach_task_self(), (int)Base, Size);
    if( err )
    {
	error(0, err, "xf86UnMapVidMem: can't dealloc framebuffer space");
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

#include <sys/io.h>

int
xf86EnableIO()
{
    if (ioperm(0, 0xffff, 1)) {
	error(1, errno, "xf86EnableIO: ioperm() failed");
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

