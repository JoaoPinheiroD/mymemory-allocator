#include <stdio.h>
#include <stdlib.h>
#include "mymemory.h"

int main()
{
	// aloca pool memoria
	mymemory_t *memory = mymemory_init(1000);

	/*
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
	*/

	void *a = mymemory_alloc(memory, 128);   // offset 0
	void *b = mymemory_alloc(memory, 256);   // offset 128
	void *c = mymemory_alloc(memory, 100);   // offset 384
	mymemory_display(memory);                // 3 nós: 0, 128, 384

	mymemory_free(memory, b);                // remove o do meio
	mymemory_display(memory);                // 2 nós: 0 e 384

	void *d = mymemory_alloc(memory, 200);   // reaproveita a lacuna: offset 128
	mymemory_display(memory);

	mymemory_free(memory, a);                // remove a cabeça
	void *e = mymemory_alloc(memory, 100);   // lacuna do início: offset 0
	mymemory_display(memory);

	mymemory_free(memory, c);                // remove o último
	mymemory_display(memory);
	mymemory_cleanup(memory);

	return 0;
}
