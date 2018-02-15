#include "pool.h"
#include "value.h"

#include <stdlib.h>

int pool_init(struct pool *p, size_t len) {
	pool_index obj_cells;

	p->cells = malloc(len * sizeof(POOL_OBJECT_TYPE));
	if (!p->cells) return 1;

	obj_cells = sizeof(POOL_OBJECT_TYPE) / sizeof(pool_index);
	for (size_t i = 0; i < len; i+=obj_cells)
		p->cells[i] = i + obj_cells;

	p->n_free = p->cells;
	return 0;
}

void pool_destroy(struct pool *p) {
	if (p) free(p->cells);
}

void *pool_alloc(struct pool *p) {
	pool_index n;
	n = p->n_free;
	p->n_free = p->cells[n];
	return (p->cells+n);
}

void pool_free(struct pool *p, void *o) {
	pool_index i = (pool_index*)o-p->cells;
	p->cells[i] = p->n_free;
	p->n_free = i;
}
