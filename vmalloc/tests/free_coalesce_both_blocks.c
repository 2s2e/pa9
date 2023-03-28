/* some allocations of zero/negative memory*/
#include "vmlib.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

int main()
{
    vminit(4096);

    void *a = vmalloc(1000);
    void *b = vmalloc(1000);

    vmfree(a);
    vmfree(b);
    
    void *c = vmalloc(4000);

    vminfo();

    assert(c != NULL);

    vmdestroy();
}
