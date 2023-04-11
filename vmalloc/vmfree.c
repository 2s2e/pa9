#include "vm.h"
#include "vmlib.h"
#include <stdio.h>

/**
 * The vmfree() function frees the memory space pointed to by ptr,
 * which must have been returned by a previous call to vmmalloc().
 * Otherwise, or if free(ptr) has already been called before,
 * undefined behavior occurs.
 * If ptr is NULL, no operation is performed.
 * vmfree() asserts that ptr is 8-byte aligned.
 */

void set_this_free(struct block_header* block_ptr) {
    block_ptr->size_status &= ~(0b1);
}

int is_occupied(struct block_header* block_ptr) {
    return block_ptr->size_status & 0b1;
}

int f_check_end(struct block_header *header) {
    if((header->size_status >> 3 == 0) && ((header->size_status & 1U) == 1)) {
        return 1;
    }
    return 0;
}

int f_check_busy(struct block_header *header) {
    if((header->size_status & (0b1)) == 1) {
        return 1;
    }
    return 0;
}

int f_check_prev_busy(struct block_header *header) {
    if((header->size_status & (0b10)) == 2) {
        return 1;
    }
    return 0;
}

void f_alloc_block(struct block_header *header, size_t true_size) {
    //this block is now occupied 
    header->size_status |= 0b1;

    //reset the size of our block
    header->size_status &= (0b111);
    header->size_status |= true_size;
}

void f_set_next_block(struct block_header *header, size_t leftover) {
    if(leftover == 0) {
        //we used the entire free space, therefore just set it to occupied
        header->size_status |= 0b10;
    }
    else {
        //there is more space left, create a new free block
        header->size_status |= leftover;
        header->size_status |= 0b10;
    }
}

void f_set_footer(struct block_header *header) {
    struct block_footer *footer = (struct block_footer*)header + (BLKSZ(header)/4 - 1);
    footer->size = BLKSZ(header);
}

void vmfree_normal(void *ptr)
{
    struct block_header* block_ptr = (struct block_header*)ptr;
    block_ptr -= 1;

    //NULL
    if(block_ptr->size_status == 0) {
        return;
    }

    //block is already free
    if(is_occupied(block_ptr) == 0) {
        return;
    }

    //free our current block
    size_t cur_ptr_size = BLKSZ(block_ptr);
    set_this_free(block_ptr);

    //next block header
    struct block_header* next_block = block_ptr + (cur_ptr_size / 4);

    //if the next block is free, coalesce it
    if(!f_check_end(next_block) && !f_check_busy(next_block)) {
        //printf("%d\n", block_ptr->size_status);
        block_ptr->size_status += BLKSZ(next_block);
        //printf("%d\n", block_ptr->size_status);
        f_set_footer(block_ptr);
    }
    else if(!f_check_end(next_block)){
        next_block->size_status &= ~(0b10);
    }

    //bug fix, assumed cur_ptr_size would stay constant, didn't take into account prev coalescing
    cur_ptr_size = BLKSZ(block_ptr);

    //if the previous block is free, coalesce it
    if(!f_check_prev_busy(block_ptr)) {
        //get previous footer and footer size
        struct block_footer* prev_footer = (struct block_footer*)block_ptr - 1;
        size_t prev_size = prev_footer->size;

        //get the header of the previous block and add the current size
        struct block_header* prev_block = block_ptr - (prev_size / 4);

        //printf("%d\n", BLKSZ(prev_block));
        prev_block->size_status += cur_ptr_size;
        //printf("%d\n", BLKSZ(prev_block));

        //set the new footer
        f_set_footer(prev_block);
        return;
    }

    f_set_footer(block_ptr);
    return;

    /* TODO: PA 6 */
}

void vmfree(struct v_pointer v) {
    struct block_header* block_ptr = v.addr;
    block_ptr -= 1;

    //let's first free our pointer on the heap
    if(BLKID(block_ptr) == v.id) {
        //printf("vmfree_normal called on %p", v.addr);
        vmfree_normal(v.addr);
    }
    else {
        //let's find the block with the proper id
        block_ptr = heapstart;
        while(block_ptr->size_status != VM_ENDMARK) {
            //printf("vmfree: current size status and block size of block pointer: %x  %d\n", block_ptr->size_status, BLKSZ(block_ptr));
            //printf("vmfree: location of block pointer: %d\n\n", block_ptr - heapstart);
            if(BLKID(block_ptr) == v.id) {
                vmfree_normal(block_ptr);
                return;
            }
            if(BLKSZ(block_ptr) == 0) {
        
                exit(1);
            }
            block_ptr += (BLKSZ(block_ptr) / 4);
        }
    }
    
    //remember, we still have to account for the chunk
    struct block_header main_ptr[1];
    

    fseek(fp, 0, SEEK_SET);
    
    while(1) {
        //check for reaching end
        int c = fgetc(fp);
        if(c == EOF) {
            break;
        }
        fseek(fp, -1, SEEK_CUR);


        fread(main_ptr, sizeof(struct block_header), 1, fp);
        fseek(fp, -4, SEEK_CUR);

        //now main_ptr contains the block header stored in the file

        char id = BLKID(main_ptr);
        size_t size = BLKSZ(main_ptr);

        char id_zero[1];

        id_zero[0] = '\0';
        
        //the file to evict already exists on the swapfile, update the contents
        if(id == v.id) {
            //get in position to modify the id
            fseek(fp, 3, SEEK_CUR);

            fwrite(id_zero, 1, 1, fp);
            return;
        }

        fseek(fp, size, SEEK_CUR);
    }
    
}

