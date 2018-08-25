#ifndef FIBERS_CONST_H
#define FIBERS_CONST_H

#include <linux/ioctl.h>

// the file created in /dev is a misc char device 
#define MAJOR_NUM 10
#define DEVICE_NAME "fibers"   /* Dev name as it appears in /proc/devices   */

#define IOCTL_TO_FIB _IOW(MAJOR_NUM, 0, int)
#define IOCTL_CREATE_FIB _IOW(MAJOR_NUM, 1, void*) // TODO: change third arg type
#define IOCTL_SWITCH_FIB _IOW(MAJOR_NUM, 2, void*)
#define IOCTL_FLS_ALLOC _IOR(MAJOR_NUM, 3, int)
#define IOCTL_FLS_FREE _IOWR(MAJOR_NUM, 4, long)
#define IOCTL_FLS_SET _IOR(MAJOR_NUM, 5, void*) // TODO: change third arg type
#define IOCTL_FLS_GET _IOWR(MAJOR_NUM, 6, long)


#endif