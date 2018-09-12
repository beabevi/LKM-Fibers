#include <stdio.h>
#include "../include/lib/fibers.h"

fid_t p, c, n;

int main() {
    p = to_fiber();
    printf("Parent fid=%lu\n", p);
    int ret = fork();
    if (ret == 0) {
        c = to_fiber();
        printf("Child fid=%lu\n", c);
        ret = fork();
        if (ret == 0) {
            n = to_fiber();
            printf("Nephew fid=%lu\n", n);
        }
    }
    return 0;
}