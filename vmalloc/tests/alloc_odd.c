/* a few allocations of odd sizes*/
#include "vmlib.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

int main()
{
    vminit(4096);

    int *ptr[5];

    ptr[0] = (int *)vmalloc(5);
    ptr[1] = (int *)vmalloc(1);
    ptr[2] = (int *)vmalloc(43);
    ptr[3] = (int *)vmalloc(77);
    ptr[4] = (int *)vmalloc(101);

    for (int i = 0; i < 5; i++) {
        assert(ptr[i] != NULL);
        assert(((uint32_t)ptr[i]) % 8 == 0);
    }

    vmdestroy();
}
