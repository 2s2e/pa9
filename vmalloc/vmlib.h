/**
 * CSE 30: Computer Organization & Systems Programming
 * Winter Quarter, 2023
 * Programming Assignment V & VI
 *
 * vmlib.h
 * This header file defines the *public* interface for our memory allocation
 * system.
 *
 * Author: Jerry Yu <jiy066@ucsd.edu>
 * February 2023
 */

#ifndef VMLIB_H
#define VMLIB_H

#include <stddef.h>
#include "vm.h"


/* Initializes an empty virtual heap */
int vminit(size_t sz);
/* Destroy the mmap'd virtual heap */
void vmdestroy();

struct v_pointer vmalloc(size_t size);

void vmfree(struct v_pointer v);

/* Print out the heap structure */
void vminfo();
/* Dump the heap into a file */
void vmdump(const char *filename);
/* Load a heap from a dump file */
int vmload(const char *filename);

void* dereference(struct v_pointer v);




#endif /* VMLIB_H */
