#include "vmlib.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{
    vmload("../dumps/ref_image4");
    vminfo();
    
    void *ptr = vmalloc(300);
    vminfo();
    void *ptr2 = vmalloc(512);
    vminfo();
    void *ptr3 = vmalloc(400);
    vminfo();
    void *ptr4 = vmalloc(160);

    assert(ptr != NULL && ptr2 != NULL && ptr3 != NULL && ptr4 != NULL);

    vminfo();

    // check pointer offsets
    printf("ptr2-ptr = %u %p\n", (uint32_t)(ptr2-ptr), ptr2);
    printf("ptr3-ptr = %u %p\n", (uint32_t)(ptr3-ptr), ptr3);
    printf("ptr4-ptr = %u %p\n", (uint32_t)(ptr4-ptr), ptr4);

    assert((uint32_t)(ptr2-ptr) == 1512);
    assert((uint32_t)(ptr3-ptr) == 2032);
    assert((uint32_t)(ptr4-ptr) == 304);

    vmdestroy();
}
