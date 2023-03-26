/**
 * CSE 30: Computer Organization & Systems Programming
 * Winter Quarter, 2023
 * Programming Assignment V & VI
 *
 * vminit.c
 * This file contians functions for setting up and tearing down the virutal heap
 * simulator used for our memory allocation system.
 * It also defines the `heapstart` global pointer that points to the very first
 * block header.
 *
 * Author: Jerry Yu <jiy066@ucsd.edu>
 * February 2023
 */

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "vm.h"
#include "vmlib.h"

#define dbprintf(fmt, args...) fprintf(stderr, "%s: " fmt, __func__, ##args)

/* Global pointer to the start of the heap */
struct block_header *heapstart = NULL;
static void *_vminit_mmap_start = NULL;
static size_t _vminit_mmap_size;

/**
 * Round up heap size to multiples of page size.
 */
static size_t get_heap_size(size_t requested_size)
{
    size_t pagesize = getpagesize();
    size_t padsize = requested_size % pagesize;
    padsize = (pagesize - padsize) % pagesize;

    return requested_size + padsize;
}

/**
 * Allocate simulated heap memory by mapping zero-bytes from /dev/zero.
 */
static void *create_heap(size_t sz)
{
    void *ptr;

    int fd = open("/dev/zero", O_RDWR);
    if (fd < 0)
        return NULL;

    ptr = mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    close(fd);
    if (ptr == MAP_FAILED)
        return NULL;

    return ptr;
}

static void *mmap_heap(const char *filename, size_t sz)
{
    void *ptr;

    int fd = open(filename, O_RDWR);
    if (fd < 0)
        return NULL;

    ptr = mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    close(fd);
    if (ptr == MAP_FAILED)
        return NULL;

    return ptr;
}

/**
 * Set up the heap's internal structure.
 * I.e. initialize the entire heap as one big free block.
 */
static struct block_header *init_heap(void *start, size_t sz)
{
    /**
     * Heap diagram:
     *
     *     |<<<---------------- FREE MEMORY SIZE --------------->>>|
     * +---+---+-----------------------------------------------+---+---+
     * | A | B | ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ | C | D |
     * +---+---+-----------------------------------------------+---+---+
     *     |   |                                               |   |
     *     |   ^                                               |   |
     *     |   ptr: the start of allocatable memory            |   |
     *     |                                                   ^   |
     *     ^                       the footer for the first block  |
     *     the first block header                                  ^
     *                                                   the end mark
     * Metadata blocks (32 bits):
     *   A - skipped for double-word alignment
     *   B - The first and only free block header.
     *   C - The block footer for the only block.
     *   D - The end marker (a busy block header with 0 size)
     *
     * As a result, ptr is the first allocatable address.
     */

    /*
     * The first block header is allocated at offset +4, so that
     * the first actual address returned to user is double-word
     * (8-byte) aligned.
     */
    char *header_addr = (char *)start + sizeof(struct block_header);
    char *end_addr = start + sz;
    char *endmark_addr = end_addr - sizeof(struct block_header);
    char *footer_addr = endmark_addr - sizeof(struct block_footer);

    struct block_header *header = (struct block_header *)header_addr;
    struct block_header *endmark = (struct block_header *)endmark_addr;
    struct block_footer *footer = (struct block_footer *)footer_addr;

    /*
     * Actual free memory is from the first block header to
     * the end mark header. (See diagram above.)
     */
    size_t free_sz = endmark_addr - header_addr;

    header->size_status = free_sz;
    header->size_status |= 1U << 1; /* Set previous block bit as busy */
    header->size_status &= ~1U;     /* Set own status bit as free */

    footer->size = free_sz;

    endmark->size_status = VM_ENDMARK;

    return header;
}

void vmdestroy()
{
    munmap(_vminit_mmap_start, _vminit_mmap_size);
    _vminit_mmap_size = 0;
    _vminit_mmap_start = NULL;
}

void arch_assert()
{
    /*
     * System architecture sanity check:
     * headers should be one 32-bit word.
     */
    assert(sizeof(struct block_header) == 4);
    assert(sizeof(struct block_footer) == 4);
}

int vminit(size_t sz)
{
    arch_assert();

    if (_vminit_mmap_start) {
        dbprintf("Error: vminit called more than once.\n");
        return -1;
    }
    if (sz <= 0) {
        dbprintf("Error: vminit received invalid size.\n");
        return -1;
    }

    sz = get_heap_size(sz);
    _vminit_mmap_size = sz;
    _vminit_mmap_start = create_heap(sz);
    if (!_vminit_mmap_start) {
        dbprintf("Error: vminit failed to create the heap.\n");
        return -1;
    } else {
        dbprintf("heap created at %p (%u bytes).\n", _vminit_mmap_start, _vminit_mmap_size);
    }
    heapstart = init_heap(_vminit_mmap_start, _vminit_mmap_size);
    dbprintf("heap initialization done.\n");
    return _vminit_mmap_size;
}

int vmload(const char *filename)
{
    arch_assert();

    if (_vminit_mmap_start || _vminit_mmap_size) {
        dbprintf("Error: vminit called more than once.\n");
        return -1;
    }

    struct stat st;
    stat(filename, &st);

    size_t pagesize = getpagesize();
    if (st.st_size % pagesize != 0) {
        dbprintf("Error: dump file size not multiple(s) of page size %d\n", pagesize);
        return -1;
    }

    _vminit_mmap_size = st.st_size;
    _vminit_mmap_start = mmap_heap(filename, _vminit_mmap_size);
    if (!_vminit_mmap_start) {
        dbprintf("Error: failed to map the heap from %s.\n", filename);
        return -1;
    } else {
        dbprintf("heap created at %p (%u bytes).\n", _vminit_mmap_start, _vminit_mmap_size);
    }
    heapstart = (struct block_header *)((char *)_vminit_mmap_start + sizeof(struct block_header));
    dbprintf("heap initialization done.\n");
    return _vminit_mmap_size;
}

void vmdump(const char *filename)
{
    if (!_vminit_mmap_start) {
        dbprintf("Error: no heap mounted.\n");
        return;
    }
    FILE *fp = fopen(filename, "w+");
    fwrite(_vminit_mmap_start, sizeof(char), _vminit_mmap_size, fp);
    fclose(fp);
}
