#pragma once
#ifndef MANAGED_ALLOC_LIBRARY
#define MANAGED_ALLOC_LIBRARY
	#include <stdlib.h>
	#include <string.h>
	#include <windows.h>
	#define ALLOC_BUCKET_DEFAULT 32

	struct managed_alloc {
		HANDLE mutex;
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
		managedalloc_source.mutex = CreateMutex(NULL, FALSE, NULL);
	}

	/// <summary>Free's up the entire managed memory system.</summary>
	void xunmanaged() {
		WaitForSingleObject(managedalloc_source.mutex, INFINITE);
		
		for (size_t i = 0; i < managedalloc_source.count; i++)
			if (managedalloc_source.alloc[i] != NULL)
				free(managedalloc_source.alloc[i]);

		free(managedalloc_source.alloc);
		free(managedalloc_source.sizes);
		managedalloc_source.alloc = NULL;
		managedalloc_source.sizes = NULL;
		managedalloc_source.length = 0;
		managedalloc_source.count = 0;
		managedalloc_source.init = 0;

		ReleaseMutex(managedalloc_source.mutex);
	}

	/// <summary>calloc()/malloc()'s a block of managed memory. If raw==0, calloc, else, malloc.</summary>
	void* xalloc(size_t length, size_t typesize, int raw) {
		size_t pntrlen = length * typesize;
		if (pntrlen == 0) return NULL;
		WaitForSingleObject(managedalloc_source.mutex, INFINITE);

		bool_t resize = false;
		size_t alloclength = 0;
		if (managedalloc_source.count >= managedalloc_source.length) {
			alloclength = managedalloc_source.length * 2;
			resize = true;
		} else if (managedalloc_source.count < (managedalloc_source.length / 2)) {
			alloclength = managedalloc_source.length / 2;
			resize = (alloclength > ALLOC_BUCKET_DEFAULT);
			alloclength = (alloclength > ALLOC_BUCKET_DEFAULT)? alloclength : ALLOC_BUCKET_DEFAULT;
		}

		if (resize) {
			void** newalloc = (void**) calloc(alloclength, sizeof(void*));
			size_t* newsizes = (size_t*) calloc(alloclength, sizeof(size_t));
			
			if (newalloc == NULL || newsizes == NULL) {
				free(newalloc);
				free(newsizes);

				ReleaseMutex(managedalloc_source.mutex);
				return NULL;
			}

			memcpy(newalloc, &managedalloc_source.alloc, managedalloc_source.count * sizeof(void*));
			memcpy(newsizes, &managedalloc_source.sizes, managedalloc_source.count * sizeof(size_t));
			free(managedalloc_source.alloc);
			free(managedalloc_source.sizes);
			managedalloc_source.alloc = newalloc;
			managedalloc_source.sizes = newsizes;
			managedalloc_source.length  = alloclength;
		}
		
		size_t position = managedalloc_source.count;
		managedalloc_source.alloc[position] = (raw==0)?calloc(length, typesize):malloc(pntrlen);
		managedalloc_source.sizes[position] = pntrlen;
		managedalloc_source.count ++;

		ReleaseMutex(managedalloc_source.mutex);
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
		WaitForSingleObject(managedalloc_source.mutex, INFINITE);
		
		for (size_t i = 0; i < managedalloc_source.count; i++) {
			if (managedalloc_source.alloc[i] == alloc) {
				void* newmem = realloc(alloc, length);
				if (newmem != NULL) {
					managedalloc_source.alloc[i] = newmem;
					managedalloc_source.sizes[i] = length;
				}

				ReleaseMutex(managedalloc_source.mutex);
				return newmem;
			}
		}

		ReleaseMutex(managedalloc_source.mutex);
		return xalloc(length, sizeof(char), 1);
	}

	/// <summary>free()'s a block of managed memory.</summary>
	void xfree(void* alloc) {
		if (alloc == NULL) return;
		WaitForSingleObject(managedalloc_source.mutex, INFINITE);

		for (size_t i = 0; i < managedalloc_source.count; i++) {
			if (managedalloc_source.alloc[i] == alloc) {
				free(alloc);

				for (size_t j = i; j < managedalloc_source.count-1; j++) {
					managedalloc_source.alloc[j] = managedalloc_source.alloc[j+1L];
					managedalloc_source.sizes[j] = managedalloc_source.sizes[j+1L];
				}

				break;
			}
		}

		managedalloc_source.count --;
		managedalloc_source.alloc[managedalloc_source.count] = NULL;
		managedalloc_source.sizes[managedalloc_source.count] = 0;
		
		ReleaseMutex(managedalloc_source.mutex);
	}

	/// <summary>Returns the byte-length of an allocation.</summary>
	size_t xlength(void* alloc) {
		WaitForSingleObject(managedalloc_source.mutex, INFINITE);

		size_t length = 0;
		for (size_t i = 0; i < managedalloc_source.count; i++)
			if (managedalloc_source.alloc[i] == alloc) {
				length = managedalloc_source.sizes[i];
				break;
			}
		
		ReleaseMutex(managedalloc_source.mutex);
		return length;
	}

	/// <summary>Returns total active pointer allocations.</summary>
	size_t xallocations() {
		WaitForSingleObject(managedalloc_source.mutex, INFINITE);
		size_t count = managedalloc_source.count;
		ReleaseMutex(managedalloc_source.mutex);
		return count;
	}

#endif
