#ifndef Q4_POOL_H
#define Q4_POOL_H

#include <stdint.h>

struct q4_cons;
#define POOL_OBJECT_TYPE struct q4_cons

typedef uint32_t pool_index;

struct pool {
	pool_index *cells;
	pool_index n_free;
};

int   pool_init(struct pool *p, size_t len);
void  pool_destroy(struct pool *p);
void *pool_alloc(struct pool *p);
void  pool_free(struct pool *p, void *o);

#endif
