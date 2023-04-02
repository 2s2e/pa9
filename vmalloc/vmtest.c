#include <stdio.h>
#include "vmlib.h"
#include "assert.h"

// Does a single allocation
// void custom_heap1()
// {
//     vminit(4096);
//     void *ptr = vmalloc(128);
//     vmdump("dumps/image1");
//     vmdestroy();
// }


void dump_ref(char *ref_dump)
{
    // load in a reference heap dump and print it out with vminfo();
    vmload(ref_dump);
    vminfo();
    vmdestroy();
}

void test1() {
    /**
     * Generate images and compare with reference.
     */
    // comment these out if you just want to test on your own
    // gen_images();
    // dump_ref("dumps/ref_image6"); // change to correspond to the heap dump you want

    /* Write your own tests here */
    // initialize the vm: 
    // size must be a multiple of 4096 due to how the OS allocates memory
    vminit(4096);
    // comment out vminit() above and uncomment following line to load a heap dump instead of
    // initializating an empty heap 
    //vmload("dumps/ref_image2");

    struct v_pointer ptr = vmalloc(2000); // try calling vmalloc once.

    struct v_pointer ptr2 = vmalloc(2000);

    char* p2 = dereference(ptr);
    p2[0] = 'A';

    struct v_pointer ptr3 = vmalloc(2000);

    vminfo(); // print out how the heap looks like at this point in time for easy visualization

    vmdump("dumps/s_image");

    vmdestroy(); // frees all memory allocated by vminit() or vmload()
}

void test2() {
    vminit(4096);

    struct v_pointer ptr = vmalloc(1000);
    struct v_pointer ptr2 = vmalloc(1000);
    struct v_pointer ptr3 = vmalloc(1000);
    struct v_pointer ptr4 = vmalloc(1000);

    struct v_pointer ptr5 = vmalloc(4000);

    assert(ptr5.addr != NULL);

    vminfo();
}

void test3() {
    vminit(4096);

    struct v_pointer ptr = vmalloc(1000);
    char* p = dereference(ptr);
    p[0] = 'A';
    struct v_pointer ptr2 = vmalloc(1000);
    struct v_pointer ptr3 = vmalloc(1000);
    struct v_pointer ptr4 = vmalloc(1000);

    struct v_pointer ptr5 = vmalloc(4000);
    char* p5 = dereference(ptr5);
    p5[0] = 'B';

    assert(ptr5.addr != NULL);

    p = dereference(ptr);
    assert(p[0] == 'A');

    vminfo();
}

int main()
{
    test2();
    return 0;
}
