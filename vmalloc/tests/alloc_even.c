/* a few allocations of even sizes */
#include "vmlib.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

int main()
{
    vminit(1024);

    int *ptr[5];

    ptr[0] = (int *)vmalloc(2);
    ptr[1] = (int *)vmalloc(6);
    ptr[2] = (int *)vmalloc(900);
    ptr[3] = (int *)vmalloc(80);
    ptr[4] = (int *)vmalloc(12);

    for (int i = 0; i < 5; i++) {
        assert(ptr[i] != NULL);
        assert(((uint32_t)ptr[i]) % 8 == 0);
    }

    vmdestroy();
}
