/* Tests free at the end of the heap when coalescing is required for the previous block */
#include "vmlib.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

int main()
{
    vmload("../dumps/ref_image6");

    // This is just to get the address of the start of the heap
    void *probe = vmalloc(260); // allocate into the first free block
    void *start = (char *)probe - 144;
    vmfree(probe);

    vmfree((char *)start + 3144);
    void *ptr1 = vmalloc(1468);
    assert(ptr1 != NULL);
    assert((uint32_t)(ptr1 - start) == 2624);

    vmdestroy();
}

