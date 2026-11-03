#include <stdio.h>
#include <stdlib.h>
#include "mymemory.h"

int main()
{
	// aloca pool memoria
	mymemory_t *memory = mymemory_init(1000);

	void *a = mymemory_alloc(memory, 128);   // 0
	void *b = mymemory_alloc(memory, 256);   // 128
	void *c = mymemory_alloc(memory, 700);   // NULL
	void *d = mymemory_alloc(memory, 616);   // 384
	void *e = mymemory_alloc(memory, 1);     // NULL (pool cheio)
	
	printf("a: %td\n", a ? (char*)a - (char*)memory->pool : -1);
	printf("b: %td\n", b ? (char*)b - (char*)memory->pool : -1);
	printf("c: %td\n", c ? (char*)c - (char*)memory->pool : -1);
	printf("d: %td\n", d ? (char*)d - (char*)memory->pool : -1);
	printf("e: %td\n", e ? (char*)e - (char*)memory->pool : -1);

	mymemory_cleanup(memory);

	return 0;
}
