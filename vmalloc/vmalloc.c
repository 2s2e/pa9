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

void alloc_block(struct block_header *header, size_t true_size, char id) {
    //this block is now occupied 
    header->size_status |= 0b1;

    //reset the size of our block
    header->size_status &= (0b111);
    header->size_status |= true_size;

    size_t int_id = id;

    int_id = int_id << 24;

    header->size_status |= int_id;

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

void copy_to_file(struct block_header *header) {
    //place the file pointer to the proper place first before calling this function
    fwrite(header, 1, BLKSZ(header), fp);
}

void add_block_to_file(struct block_header *header) {
    //start at the beginning, keep going until you reach the end, then stop

    //our blockhead buffer
    struct block_header main_ptr[1];
    

    fseek(fp, 0, SEEK_SET);

    while(!feof(fp)) {
        fread(main_ptr, sizeof(struct block_header*), 1, fp);
        printf("%x\n", main_ptr->size_status);
        char id = BLKID(main_ptr);
        size_t size = BLKSZ(main_ptr);
        
        //the file to evict already exists on the swapfile, update the contents
        if(id == BLKID(header)) {
            fseek(fp, -4, SEEK_CUR);
            copy_to_file(header);
            return;
        }
        fseek(fp, size-4, SEEK_CUR);
    }
    //the file to exist is new, put it on the heap
    copy_to_file(header);
}


struct v_pointer swap_alloc(size_t size) {
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

            //is our current block big enough?
            if(BLKSZ(main_ptr)-4 >=size && BLKSZ(main_ptr) < min_free_size) {
                //printf("%x is free",main_ptr->size_status);
                min_free_size = BLKSZ(main_ptr);
                min_free_header = main_ptr;
            }
        

        //# of 4 byte increments between here and the next block
        int jump_size = (BLKSZ(main_ptr) >> 2);
        main_ptr = (main_ptr+jump_size);   
    }

    //-----------------------------------------------------------------
    //We will now begin the process of evicting our block

    struct block_header *to_evict = min_free_header;
    int eviction_size = min_free_size;

    //our largest block isn't big enough, we will allocate enough space at the start
    if(to_evict == NULL || min_free_size - 4 < size) {
        to_evict = heapstart;
        eviction_size = 0;

        main_ptr = to_evict;
        main_ptr += (BLKSZ(main_ptr) >> 2);


        while(eviction_size - 4 < size) {
            add_block_to_file(main_ptr);
            eviction_size += BLKSZ(main_ptr);
            main_ptr += (BLKSZ(main_ptr) >> 2);
        }
    }
    else {
        add_block_to_file(to_evict);
    }

    //the true size of our block to allocate, header + payload rounded up 8
    size_t true_size = eviction_size;

    //allocate that ****
    alloc_block(to_evict, true_size, next_id);
        
    struct v_pointer toRet;
    toRet.addr = to_evict;
    toRet.id = next_id;
    return toRet;
    
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

    if(size == 0 || size > heapsize - 8) {
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
        int jump_size = (BLKSZ(main_ptr) >> 2);
        main_ptr = (main_ptr+jump_size);   
    }

    //we do not have a free block big enough
    if(min_free_header == NULL) {
        //this is where the fun begins




        struct v_pointer toRet = swap_alloc(size);
        next_id++;
        return toRet;
    }

    //the true size of our block to allocate, header + payload rounded up 8
    size_t true_size = ROUND_UP(4+size, 8);

    //allocate that ****
    alloc_block(min_free_header, true_size, next_id);

    //have a reference point to the next free area
    struct block_header *next_header = (min_free_header+(true_size >> 2));
    size_t leftover = min_free_size - true_size;
    //properly clean up the next block
    set_next_block(next_header, leftover);

    //printf("%p", min_free_size);
    //random comments

    struct v_pointer toRet;
    toRet.addr = min_free_header+1;
    toRet.id = next_id;

    next_id++;
    return toRet;
}
