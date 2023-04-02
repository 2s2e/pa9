#include "vm.h"
#include "vmlib.h"
#include <stdio.h>

/**
 * The malloc() function allocates size bytes and returns a pointer
 * to the allocated memory (payload). The memory is not initialized.
 * If size is 0, or if no available heap memory could be allocated,
 * malloc returns NULL.
 */

char next_id = 1;



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


    //set the id of the block
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

void set_footer(struct block_header *header) {
    struct block_footer *footer = (struct block_footer*)header + (BLKSZ(header)/4 - 1);
    footer->size = BLKSZ(header);
}

void coalesce_next(struct block_header* block_ptr)
{
    //NULL
    if(block_ptr->size_status == 0) {
        return;
    }
    if(check_busy(block_ptr)) {
        return;
    }

    size_t cur_ptr_size = BLKSZ(block_ptr);

    //next block header
    struct block_header* next_block = block_ptr + (cur_ptr_size / 4);

    //if the next block is free, coalesce it
    if(!check_end(next_block) && !check_busy(next_block)) {
        //printf("%d\n", block_ptr->size_status);
        block_ptr->size_status += BLKSZ(next_block);

        //printf("%d\n", block_ptr->size_status);
        set_footer(block_ptr);
    }
    else if(!check_end(next_block)){
        next_block->size_status &= ~(0b10);
    }

    set_footer(block_ptr);
    return;

    /* TODO: PA 6 */
}

void copy_to_file(struct block_header *header) {
    //place the file pointer to the proper place first before calling this function
    fwrite(header, 1, BLKSZ(header), fp);
}

void add_block_to_file(struct block_header *header) {
    //start at the beginning, keep going until you reach the end, then stop
    printf("Adding block to file of size %d\n", BLKSZ(header));
    //our blockhead buffer
    struct block_header main_ptr[1];
    

    fseek(fp, 0, SEEK_SET);
    printf("%x adding block to file\n", header->size_status);
    
    while(1) {
        //check for reaching end
        int c = fgetc(fp);
        if(c == EOF) {
            break;
        }
        fseek(fp, -1, SEEK_CUR);


        fread(main_ptr, sizeof(struct block_header), 1, fp);
        if(main_ptr->size_status == NULL) {
            break;
        }
        printf("reading %x, size %d \n", main_ptr->size_status, BLKSZ(main_ptr));

        fseek(fp, -4, SEEK_CUR);
        char id = BLKID(main_ptr);
        size_t size = BLKSZ(main_ptr);
        
        //the file to evict already exists on the swapfile, update the contents
        if(id == BLKID(header)) {
            copy_to_file(header);
            return;
        }
        fseek(fp, size, SEEK_CUR);
    }
    //the file to exist is new, put it on the heap
    copy_to_file(header);
}







void* dereference(struct v_pointer v) {
    //pointer at whatever location v was originally pointing to
    struct block_header* ptr = v.addr - 1;

    if(BLKID(ptr) == v.id) {
        return v.addr;
    }

    //the original location has been invaded, we start at the beginning
    ptr = heapstart;

     //our blockhead buffer
    struct block_header header_buf[1];
    
    //start at the beginning
    fseek(fp, 0, SEEK_SET);

    while(!feof(fp)) {
        fread(header_buf, sizeof(struct block_header*), 1, fp);
        printf("%x\n", header_buf->size_status);

        char id = BLKID(header_buf);
        size_t size = BLKSZ(header_buf);
        
        //we've found our header
        if(id == v.id) {
            break;
        }
        //we've reached the end, and we haven't found our header, therefore it no longer exists
        if(id == 0) {
            return NULL;
        }
        fseek(fp, size-4, SEEK_CUR);
    }


    size_t size = BLKSZ(header_buf);

    //first, we will evict all the necessary blocks to make space
    struct block_header* to_evict = ptr;
    size_t eviction_size = 0;

    //initialize our main pointer
    struct block_header* main_ptr = to_evict;
    //main_ptr += (BLKSZ(main_ptr) >> 2);

    //while the space we've evicted isn't big enough, keep on evicting
    while(eviction_size == 0 || eviction_size - 4 < size) {
        if(check_busy(main_ptr)) {
            add_block_to_file(main_ptr);
        }
        eviction_size += BLKSZ(main_ptr);
        main_ptr += (BLKSZ(main_ptr) >> 2);
    }

    //allocates our block
    alloc_block(to_evict, BLKSZ(header_buf), BLKID(header_buf));
    size_t leftovers = eviction_size - size;
    struct block_header *next_header = (to_evict+(size >> 2));

    //set our next block and coalesce
    set_next_block(next_header, leftovers);
    coalesce_next(next_header);


    //let's copy over our data
    char* writer = (char*)to_evict;

    //this works, because at this point, our fp should be pointing at where our block header is
    fread(writer, size, 1, fp);


    return to_evict+1;
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
        //set our block header and block size to correspond to the start
        to_evict = heapstart;
        eviction_size = 0;

        //initialize our main pointer
        main_ptr = to_evict;
        //main_ptr += (BLKSZ(main_ptr) >> 2);

        //while the space we've evicted isn't big enough, keep on evicting
        while(eviction_size == 0 || eviction_size - 4 < size) {
            if(check_busy(main_ptr)) {
                add_block_to_file(main_ptr);
            }
            eviction_size += BLKSZ(main_ptr);
            main_ptr += (BLKSZ(main_ptr) >> 2);
            printf("%d swapalloc eviction size\n", eviction_size);
        }
    }
    else {
        //our largest block is big enough, so we can just evict that one block
        add_block_to_file(to_evict);
    }

    //the true size of our block to allocate, header + payload rounded up 8
    size_t true_size = ROUND_UP(4+size, 8);

    printf("%xBefore allocating\n", to_evict->size_status);
    //allocate that ****
    alloc_block(to_evict, true_size, next_id);
    printf("%x After allocating\n", to_evict->size_status);

    
    //create our struct
    struct v_pointer toRet;
    toRet.addr = to_evict+1;
    toRet.id = next_id;

    //don't forget to set up our free blocks
    size_t leftover = eviction_size - true_size;

    struct block_header *next_header = (to_evict+(true_size >> 2));
    set_next_block(next_header, leftover);

    printf("%x After doing set next block\n", to_evict->size_status);

    //coalesce in case our block doesn't take up the whole space, we are coalescing the next block
    coalesce_next(next_header);

    printf("%x After doing all the fancy shit\n", to_evict->size_status);

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
    printf("%x\n", min_free_header->size_status);

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
