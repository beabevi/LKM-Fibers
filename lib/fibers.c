#include "../include/lib/fibers.h"

#define assert_fibers_open \
        do{              \
            if (__fibers_file < 0){ \
                printf("fibers file is not open"); \
            } \
        }while(0);

int __fibers_file = -1;

void to_fiber(void){
    __fibers_file = open("/dev/" DEVICE_NAME, O_NONBLOCK);
    assert_fibers_open

    printf("IOCTL_TO_FIB = %lu\n", IOCTL_TO_FIB);
    int ret = ioctl(__fibers_file, IOCTL_TO_FIB, 0);
    if (ret < 0){
        printf("Error in ioctl");
    }
}
void create_fiber(size_t stack_size, void (*entry_point)(void*), void* param){
    assert_fibers_open

    int ret = ioctl(__fibers_file, IOCTL_CREATE_FIB, 0);
    if (ret < 0){
        printf("Error in ioctl");
    }
}
void switch_fiber(void* fid){
    assert_fibers_open

    int ret = ioctl(__fibers_file, IOCTL_SWITCH_FIB, fid);
    if (ret < 0){
        printf("Error in ioctl");
    }
}
long fls_alloc(void){
    assert_fibers_open

    int ret = ioctl(__fibers_file, IOCTL_FLS_ALLOC, 0);
    if (ret < 0){
        printf("Error in ioctl");
    }
}
bool fls_free(long index){
    assert_fibers_open

    int ret = ioctl(__fibers_file, IOCTL_FLS_FREE, index);
    if (ret < 0){
        printf("Error in ioctl");
    }
}
void fls_set(long index, long long value){
    assert_fibers_open

    int ret = ioctl(__fibers_file, IOCTL_FLS_SET, 0);
    if (ret < 0){
        printf("Error in ioctl");
    }
}
long long fls_get(long index){
    assert_fibers_open

    int ret = ioctl(__fibers_file, IOCTL_FLS_GET, index);
    if (ret < 0){
        printf("Error in ioctl");
    }
}