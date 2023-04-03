#include <stdio.h>
#include "vmlib.h"
#include "assert.h"

// Does a single allocation
// void custom_heap1()
// {
//     vminit(4096);
//     void *ptr = vmalloc(128);
//     vmdump("dumps/image1");
//     vmdestroy();
// }


void dump_ref(char *ref_dump)
{
    // load in a reference heap dump and print it out with vminfo();
    vmload(ref_dump);
    vminfo();
    vmdestroy();
}

//sanity check test, tests if dereferencing on a v_pointer that doesn't go into the file works
void test1() {

    vminit(4096);

    struct v_pointer v_ptr = vmalloc(4); // try calling vmalloc once.

    int* p = dereference(v_ptr);
    *p = 5;

    int* p2 = dereference(v_ptr);
    assert(*p2 == 5);

    vminfo(); 
    vmdestroy(); 
}

//checks if the program is able to allocate space for a block that wouldn't fit onto the heap
void test2() {
    vminit(4096);

    struct v_pointer ptr = vmalloc(1000);
    struct v_pointer ptr2 = vmalloc(1000);
    struct v_pointer ptr3 = vmalloc(1000);
    struct v_pointer ptr4 = vmalloc(1000);
    printf("%x Size status before calling vmalloc \n", BLKSZ(heapstart));
    struct v_pointer ptr5 = vmalloc(4000);
    printf("%x Size status after \n", BLKSZ(heapstart));

    assert(ptr5.addr != NULL);

    vminfo();
    vmdestroy();
    printf("(:");
}

//checks if content in evicted blocks can still be accessed 
void test3() {
    vminit(4096);

    struct v_pointer ptr = vmalloc(1000);
    //printf("%x size status of newly allocated pointer \n\n", ((struct block_header*)(ptr.addr)-1)->size_status);

    
    char* p = dereference(ptr);
    //printf("%p Value of dereferencing ptr\n", p);
    //printf("%d Value of v pointer address\n",((int*)ptr.addr)[0]);
    p[0] = 'A';
    struct v_pointer ptr2 = vmalloc(1000);
    struct v_pointer ptr3 = vmalloc(1000);
    struct v_pointer ptr4 = vmalloc(1000);

    struct v_pointer ptr5 = vmalloc(4000);
    char* p5 = dereference(ptr5);
    p5[0] = 'B';

    assert(ptr5.addr != NULL);

    p = dereference(ptr);
    printf("p[0] = %c\n", p[0]);

    assert(p[0] == 'A');

    vminfo();

    vmdestroy();

}

//more advanced testing to see if dereferencing works after multiple evictions for a single vmalloc
void test4() {
    vminit(4096);

    char* p;

    struct v_pointer ptr = vmalloc(1000);
    
    p = dereference(ptr);
    p[0] = 'A';

    struct v_pointer ptr2 = vmalloc(1000);

    p = dereference(ptr2);
    p[0] = 'B';

    struct v_pointer ptr3 = vmalloc(1000);

    p = dereference(ptr3);
    p[0] = 'C';

    struct v_pointer ptr4 = vmalloc(1000);

    p = dereference(ptr4);
    p[0] = 'D';

    vminfo();


    struct v_pointer ptr5 = vmalloc(4000);
    char* p5 = dereference(ptr5);
    
    for(int i = 0; i < 4000; i++) {
        p5[i] = 'Z';
    }

    vminfo();


    assert(ptr5.addr != NULL);

    p = dereference(ptr);
    printf("1 p[0] = %c\n", p[0]);
    assert(p[0] == 'A');

    p = dereference(ptr2);
    printf("2 p[0] = %c\n", p[0]);
    assert(p[0] == 'B');

    p = dereference(ptr3);
    printf("3 p[0] = %c\n", p[0]);
    assert(p[0] == 'C');

    p = dereference(ptr4);
    printf("4 p[0] = %c\n", p[0]);
    assert(p[0] == 'D');

    p5 = dereference(ptr5);
    for(int i = 0; i < 4000; i++) {
        assert(p5[i] == 'Z');
    }

    vminfo();

    vmdestroy();
}

void test_2d_array() {
    vminit(4096);
    //we will attempt to simulate a dynamically allocated 2d array

    struct v_pointer mat;

    mat = vmalloc(sizeof(struct v_pointer) * 5);

    struct v_pointer* mat_p = dereference(mat);
    int* arr_p; //we will use this to store the reference to each integer array

    for(int i = 0; i < 5; i++) {
        //arr is an array of integers
        mat_p[i] = vmalloc(sizeof(int) * (i+1));
        arr_p = dereference(mat_p[i]);
        for(int j = 0; j < i+1; j++) {
            arr_p[j] = j;
        }
    }

    struct v_pointer huge_block = vmalloc(4080);



    //mat_p is of type v_pointer[]
    mat_p = dereference(mat);
    struct v_pointer arr = mat_p[2];
    arr_p = dereference(arr);
    int val = arr_p[2];

    assert(val == 2);

    mat_p = dereference(mat);
    arr = mat_p[4];
    arr_p = dereference(arr);
    val = arr_p[0];

    assert(val == 0);

    vmdestroy();
}

//sanity check for a single vmfree and vmalloc
void test_free_1() {
    vminit(4096);

    struct v_pointer v_ptr = vmalloc(4);
    vmfree(v_ptr);

    struct v_pointer v_ptr2 = vmalloc(4);

    //after the first block, the heap is empty, therefore our second block should be allocated at the beginning
    assert(v_ptr2.addr == heapstart+1);

    vminfo();
    vmdestroy();
}

void test_free_2() {
    vminit(4096);

    struct v_pointer v_ptr = vmalloc(1000);

    assert(dereference(v_ptr) != NULL);

    struct v_pointer v_ptr2 = vmalloc(4000);

    assert(dereference(v_ptr) != NULL);

    vmfree(v_ptr);

    assert(dereference(v_ptr) == NULL);
    

    vminfo();
    vmdestroy();
}


int main()
{
    test_free_2();
    return 0;
}
