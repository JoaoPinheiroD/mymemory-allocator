#include <stdio.h>
#include "mymemory.h"

int main(void)
{
	mymemory_t *m = mymemory_init(1000);
	if (m == NULL) {
		return 1;
	}

	void *a = mymemory_alloc(m, 128);
	void *b = mymemory_alloc(m, 256);
	void *c = mymemory_alloc(m, 100);
	printf("--- 3 alocacoes ---\n");
	mymemory_display(m);
	mymemory_stats(m);

	mymemory_free(m, b);
	printf("--- free(b): lacuna no meio ---\n");
	mymemory_display(m);

	void *d = mymemory_alloc(m, 200); 
	mymemory_free(m, a);
	void *e = mymemory_alloc(m, 100);
	printf("--- realocacoes ---\n");
	mymemory_display(m);

	mymemory_free(m, NULL);
	mymemory_free(m, c);
	printf("--- free(c) ---\n");
	mymemory_stats(m);

	if (d == NULL || e == NULL || mymemory_alloc(m, 5000) != NULL) {
		printf("FALHA\n");
	}

	mymemory_cleanup(m);
	return 0;
}
