#define IO_INTRAN trivfs_protid_t trivfs_begin_using_protid (io_t)
#define IO_DESTRUCTOR trivfs_end_using_protid (trivfs_protid_t)

#define KGI_IMPORTS import <hurd/trivfs.h>; import "kgi/kgi.h";
