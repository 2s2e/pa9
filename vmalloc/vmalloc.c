#include "vm.h"
#include "vmlib.h"
#include <stdio.h>

/**
 * The malloc() function allocates size bytes and returns a pointer
 * to the allocated memory (payload). The memory is not initialized.
 * If size is 0, or if no available heap memory could be allocated,
 * malloc returns NULL.
 */

 /*
#define BLKSZ(b) ((b)->size_status & VM_BLKSZMASK)
#define ROUND_UP(N, S) ((((N) + (S)-1) / (S)) * (S))
 */

char next_id = 0;



int check_end(struct block_header *header) {
    if((header->size_status >> 3 == 0) && ((header->size_status & 1U) == 1)) {
        return 1;
    }
    return 0;
}

int check_busy(struct block_header *header) {
    if((header->size_status & (0b1)) == 1) {
        return 1;
    }
    return 0;
}

int check_prev_busy(struct block_header *header) {
    if((header->size_status & (0b10)) == 2) {
        return 1;
    }
    return 0;
}

void alloc_block(struct block_header *header, size_t true_size) {
    //this block is now occupied 
    header->size_status |= 0b1;

    //reset the size of our block
    header->size_status &= (0b111);
    header->size_status |= true_size;
}

void set_next_block(struct block_header *header, size_t leftover) {
    if(leftover == 0) {
        if (header->size_status != VM_ENDMARK) {
            //we used the entire free space, therefore just set it to occupied
            header->size_status |= 0b10;
        }
    }
    else {
        //there is more space left, create a new free block
        header->size_status |= leftover;
        header->size_status |= 0b10;
    }
}

void* dereference(struct v_pointer v) {
    return v.addr;
}

struct v_pointer vmalloc(size_t size)
{

    /*
    one for getting block size
    one for getting a pointer to next block
    one for setting allocation bit
    one for setting the previous status bit
    */
    if(next_id == 255) {
        printf("Maximum number of allocs reached");
        exit(1);
    }

    if(size == 0) {
        struct v_pointer toRet;
        toRet.addr = NULL;
        toRet.id = 0;
        return toRet;
    }



    //main_ptr will be our traversing pointer
    struct block_header *main_ptr = heapstart;

    //keep track of our min free size and min free block
    size_t min_free_size = 0xFFFFFFFF;
    struct block_header *min_free_header = NULL;

    //let's find our smallest free block!

    //while we haven't reached the end...
    while(!check_end(main_ptr)) {
        //debugging
        // printf("%d\n",BLKSZ(main_ptr));
        // printf("%p\n",main_ptr);

        //is our current block busy?
        if(check_busy(main_ptr) == 0) {

            //is our current block big enough?
            if(BLKSZ(main_ptr)-4 >=size && BLKSZ(main_ptr) < min_free_size) {
                //printf("%x is free",main_ptr->size_status);
                min_free_size = BLKSZ(main_ptr);
                min_free_header = main_ptr;
            }
        }
        

        //# of 4 byte increments between here and the next block
        int jump_size = (main_ptr->size_status >> 2);
        main_ptr = (main_ptr+jump_size);   
    }

    //we do not have a free block big enough
    if(min_free_header == NULL) {
        //this is where the fun beginss
        struct v_pointer toRet;
        toRet.addr = NULL;
        toRet.id = 0;
        return toRet;
    }

    //the true size of our block to allocate, header + payload rounded up 8
    size_t true_size = ROUND_UP(4+size, 8);

    //allocate that ****
    alloc_block(min_free_header, true_size);

    //have a reference point to the next free area
    struct block_header *next_header = (min_free_header+(true_size >> 2));
    size_t leftover = min_free_size - true_size;
    //properly clean up the next block
    set_next_block(next_header, leftover);

    //printf("%p", min_free_size);
    //random comments

    struct v_pointer toRet;
    toRet.addr = min_free_header+1;
    toRet.id = 0;
    return toRet;
}
