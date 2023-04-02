#include <stdio.h>

#include "vm.h"
#include "vmlib.h"

#define BLKOFF(b) ((char *)b - (char *)heapstart + 4)

void vminfo()
{
    static const char *blkstats[] = {"FREE", "BUSY"};

    struct block_header *block = heapstart;
    int blockid = 0;
    int nfree = 0, nbusy = 0;

    int busy, prev_busy;
    size_t blocksz;
    size_t totalsz = 0;

    printf("---------------------------------------\n");
    printf(" %-6s %-7s %-8s %-8s %-7s\n", "#", "stat", "offset", "size", "prev");
    printf("---------------------------------------\n");
    while (block->size_status != VM_ENDMARK) {
        blocksz = BLKSZ(block);
        busy = block->size_status & VM_BUSY ? 1 : 0;
        prev_busy = block->size_status & VM_PREVBUSY ? 1 : 0;
        printf(" %-6d %-7s %-8d %-8d %-7s\n", blockid++, blkstats[busy], BLKOFF(block), blocksz,
               blkstats[prev_busy]);

        if (busy) {
            nbusy++;
        } else {
            nfree++;
        }
        totalsz += blocksz;
        block = (struct block_header *)((char *)block + blocksz);
    }
    printf(" %-6s %-7s %-8d %-8s %-7s\n", "END", "N/A", BLKOFF(block), "N/A", "N/A");
    printf("---------------------------------------\n");
    printf("Total: %d B, Free: %d, Busy: %d, Total: %d\n", totalsz, nfree, nbusy, nfree + nbusy);
}
