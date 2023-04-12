# pa9

My attempt at the optional Programming Assignment 9 assigned in WI23 quarter's CSE 30 at UCSD.

## Usage
Usage for this custom implementation of malloc can be done in vmtest's main function.

A usage demo along with written tests can be found in the vmtest.c file.

## Implementation
* To keep track of whether or not a block is on the heap, each block allocated has a corresponding id 1-254 associated with it. This id is stored as a property in the custom struct v_pointer, and in the most significant 8 bits in the block header.
* The dereference function first checks the heap for blocks that match the id of the v_pointer being dereferenced, and then checks the swapfile.
* If there is enough space left in the heap to allocate a new block or to swap back a block from the swapfile, that space is used.
Otherwise, space is evicted starting from the beginning of the heap, moving as many blocks to the swapfile as necessary.

## Limitation
* As a result of this, there can only be 254 calls to vmalloc before the undefined behavior occurs, and thus the program is set to crash when the 255th call is made.
* Furthermore, because the last 8 bits are used to store this id, the maximum size for a block becomes 2^24 bytes. 
