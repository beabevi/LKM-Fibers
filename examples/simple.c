#include "../include/lib/fibers.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>

fid_t id1, id2, id3;
float glob_PI __attribute__((aligned(16)))= 3.14f;

void f2(void * arg){
    int i;
    int a=5, b=4, c=3, d=1;
    for (i = 0; i < (int) arg; i++){
        printf("fiber f2\n");
        printf("a = %d, b = %d, c = %d, d = %d\n", a, b, c, d);
        printf("switching to fiber %lu\n", id1);
        switch_fiber((void *) id1);
    }
    float e = 3.104f;
    printf("%f\n", e);
    switch_fiber((void*) id3);
    exit(0);
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
    register float PI;
    id2 = (fid_t) create_fiber(2<<12, f2, (void *) 1);
    id1 = (fid_t) create_fiber(2<<12, f1, (void *) 1);
    id3 = (fid_t) to_fiber();
    if (id1 == -1 || id2 == -1 || id3 == -1) {
        printf("failed to create fibers\n");
        exit(1);
    }
    switch_fiber((void*) id2);
    printf("%f\n", PI);
    PI += id1;
    switch_fiber((void*) id2);
    printf("%f\n", PI);

}
