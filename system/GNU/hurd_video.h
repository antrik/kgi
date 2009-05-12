#include <mach.h>

memory_object_t get_fb_object(vm_address_t addr, vm_size_t size, vm_prot_t prot);
void *mmio_map(void *Base, size_t Size);
void mmio_unmap(void *Base, size_t Size);
