#include "../include/lib/fibers.h"
#include <stdio.h>
#include <stdlib.h>

void bla(void){
    printf("bla was called!\n");
    exit(0);
}

int main() {
    to_fiber();
    switch_fiber(bla);
}
