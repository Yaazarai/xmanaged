# xmanaged (C)
C library will track pointer allocations to reduce potential memory leaks errors in applications.

* `xmanaged()` initializes the library.
* `xunmanaged()` un-initializes the library and free's any remaining managed allocations.
* `xalloc(count, typesize, mode)` allocates a new block of memory with a byte length of `count * typesize`. if `mode == 0` the block is zero initialized, any non-zero value will leave the block unmodified(raw).
* `xcalloc(count, typesize)` allocates a new block of memory with a byte length of `count * typesize` and zero initializes the memory.
* `xmalloc(length)` allocates a new block of memory with a byte lengbth of `length`.
* `xrealloc(pointer, length)` will attempt to resize the allocation and will return the new `pointer` on success or `NULL` on fail. If the allocation passed is `NULL` it will allocate a new raw block of memory.
* `xfree(alloc)` will free and remove the managed allocation.
* `xlength(alloc)` will return the length of the managed allocation or `0` if the allocation doesn't exist.
* `xallocations()` returns the total number of active block of memory allocations.

The example below allocates and displays the pointer's address and length, then free's two allocations and prints the remaining pointers.
```C
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "xmanaged.h"

int main() {
	xmanaged();
	srand((unsigned int)time(NULL));
	void* pointers[10];

	for (int i = 0; i < 10; i++) {
		int r = 32 + rand() % (256-32);
		pointers[i] = xalloc(r, sizeof(char), 0);
		printf("%d: %p - %zu\n", i, pointers[i], xlength(pointers[i]));
	}
	
	size_t p1 = 4, p2 = 9;
	xfree(pointers[p1]);
	xfree(pointers[p2]);

	printf("%zu, %zu - %zu\n", p1, p2, xallocations());
	for (size_t i = 0; i <  10; i++)
		printf("\t\t%zu: %p - %zu\n", i, managedalloc_source.alloc[i], xlength(managedalloc_source.alloc[i]));
	
	xunmanaged();
	printf("total: %zu\n", xallocations());
	return 0;
}
```
