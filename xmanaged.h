#pragma once
#ifndef MANAGED_ALLOC_LIBRARY
#define MANAGED_ALLOC_LIBRARY
	#include <stdlib.h>
	#include <string.h>
	#define ALLOC_BUCKET_DEFAULT 32

	struct managed_alloc {
		size_t length;
		size_t count;
		size_t init;
		void**  alloc;
		size_t* sizes;
	} managedalloc_source;

	/// <summary>Inits the managed memory system.</summary>
	void xmanaged() {
		managedalloc_source.alloc = (void**) calloc(ALLOC_BUCKET_DEFAULT, sizeof(void*));
		managedalloc_source.sizes = (size_t*) calloc(ALLOC_BUCKET_DEFAULT, sizeof(size_t));
		managedalloc_source.length = ALLOC_BUCKET_DEFAULT;
		managedalloc_source.count = 0;
		managedalloc_source.init = INT_MAX;
	}

	/// <summary>Free's up the entire managed memory system.</summary>
	void xunmanaged() {
		for (int i = 0; i < managedalloc_source.count; i++)
			free(managedalloc_source.alloc[i]);

		free(managedalloc_source.alloc);
		free(managedalloc_source.sizes);
		managedalloc_source.alloc = NULL;
		managedalloc_source.sizes = NULL;
		managedalloc_source.length = 0;
		managedalloc_source.count = 0;
		managedalloc_source.init = 0;
	}

	/// <summary>calloc()/malloc()'s a block of managed memory. If raw==0, calloc, else, malloc.</summary>
	void* xalloc(size_t length, size_t typesize, int raw) {
		size_t pntrlen = length * typesize;
		if (pntrlen == 0) return NULL;

		if (managedalloc_source.count >= managedalloc_source.length) {
			void** newalloc = (void**) calloc(managedalloc_source.count * 2L, sizeof(void*));
			size_t* newsizes = (size_t*) calloc(managedalloc_source.count * 2L, sizeof(size_t));
			
			if (newalloc == NULL || newsizes == NULL) {
				free(newalloc);
				free(newsizes);
				return NULL;
			}

			memcpy(newalloc, &managedalloc_source.alloc, managedalloc_source.count);
			memcpy(newsizes, &managedalloc_source.sizes, managedalloc_source.count);
			free(managedalloc_source.alloc);
			free(managedalloc_source.sizes);
			managedalloc_source.alloc = newalloc;
			managedalloc_source.sizes = newsizes;
			managedalloc_source.length = managedalloc_source.length * 2;
		}
		
		size_t position = managedalloc_source.count;
		managedalloc_source.alloc[position] = (raw==0)?calloc(length, typesize):malloc(pntrlen);
		managedalloc_source.sizes[position] = pntrlen;
		managedalloc_source.count ++;
		return managedalloc_source.alloc[position];
	}

	/// <summary>malloc()'s a block of managed memory.</summary>
	void* xmalloc(size_t length, size_t typesize) {
		return xalloc(length, typesize, 1);
	}

	/// <summary>calloc()'s a block of managed memory.</summary>
	void* xcalloc(size_t length, size_t typesize) {
		return xalloc(length, typesize, 0);
	}

	/// <summary>realloc()'s a block of managed memory.</summary>
	void* xrealloc(void* alloc, size_t length) {
		if (length == 0) return NULL;
		for (size_t i = 0; i < managedalloc_source.count; i++) {
			if (managedalloc_source.alloc[i] == alloc) {
				void* newmem = realloc(alloc, length);
				if (newmem != NULL) {
					managedalloc_source.alloc[i] = newmem;
					managedalloc_source.sizes[i] = length;
					return newmem;
				}
				return newmem;
			}
		}

		return xalloc(length, sizeof(char), 1);
	}

	/// <summary>free()'s a block of managed memory.</summary>
	void xfree(void* alloc) {
		if (alloc == NULL) return;
		size_t position = 0;
		int found = -1;
		for (size_t i = 0; i < managedalloc_source.count; i++) {
			if (managedalloc_source.alloc[i] == alloc) {
				found = position = i;
				break;
			}
		}
		if (found < 0) return;
		free(alloc);

		for (size_t i = position; i < managedalloc_source.count-1; i++) {
			managedalloc_source.alloc[i] = managedalloc_source.alloc[i+1L];
			managedalloc_source.sizes[i] = managedalloc_source.sizes[i+1L];
		}
		managedalloc_source.count --;
		managedalloc_source.alloc[managedalloc_source.count] = NULL;
		managedalloc_source.sizes[managedalloc_source.count] = 0;
	}

	/// <summary>Returns the byte-length of an allocation.</summary>
	size_t xlength(void* alloc) {
		for (size_t i = 0; i < managedalloc_source.count; i++)
			if (managedalloc_source.alloc[i] == alloc)
				return managedalloc_source.sizes[i];
		
		return 0;
	}

	/// <summary>Returns total active pointer allocations.</summary>
	size_t xallocations() {
		return managedalloc_source.count;
	}

#endif
