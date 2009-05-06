#include <mach.h>

memory_object_t get_fb_object(vm_address_t addr, vm_size_t size, vm_prot_t prot);

void *xf86MapVidMem(int ScreenNum,int Flags, unsigned long Base, unsigned long Size);
void xf86UnMapVidMem(int ScreenNum,void *Base,unsigned long Size);
int xf86EnableIO(void);
void xf86DisableIO(void);
