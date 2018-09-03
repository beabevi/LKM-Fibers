#include "../include/lib/fibers.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>

fid_t id1, id2, id3;

void f2(void * arg){
    int i;
    int a=5, b=4, c=3, d=1;
    for (i = 0; i < (int) arg; i++){
        printf("fiber f2\n");
        printf("a = %d, b = %d, c = %d, d = %d\n", a, b, c, d);
        printf("switching to fiber %lu\n", id1);
        switch_fiber((void *) id1);
    }
    // float e = 3.14f; // TODO: fix floating point usage failing
    // printf("e = %f\n", e);
    switch_fiber((void*) id3);
}

void f1(void * arg){
    //float a = 3.14f;
    int a = 1, b=24, c=31, d=15;
    for (int i = 0; i < (int) arg; i++) {
        printf("fiber f1\n");
        printf("a = %d, b = %d, c = %d, d = %d\n", a, b, c, d);
        printf("switching to fiber %lu\n", id2);
        switch_fiber((void *) id2);
    }
}

int main() {
/**
    pthread_t threads[3];
    int i;

    to_fiber();
    for (i = 0; i < 3; i++){
        pthread_create(&threads[i], NULL, bla, NULL);
    }
    for (i = 0; i < 3; i++){
        pthread_join(threads[i], NULL);
    }
    //switch_fiber(bla);
*/
    float PI = 3.14f;
    id2 = (fid_t) create_fiber(2<<20, f2, (void *) 3);
    id1 = (fid_t) create_fiber(2<<20, f1, (void *) 3);
    id3 = (fid_t) to_fiber();
    switch_fiber((void*) id2);
    printf("%f\n", PI);

}
