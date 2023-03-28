/* some allocations of zero/negative memory*/
#include "vmlib.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

int main()
{
    vminit(4096);

    assert(vmalloc(0) == NULL);
    assert(vmalloc(0) == NULL);
    assert(vmalloc(12) != NULL);

    vmdestroy();
}
