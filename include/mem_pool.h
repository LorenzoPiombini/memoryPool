#ifndef _MEM_POOL_H
#define _MEM_POOL_H


#define CHUNK_SIZE 1024

enum type{
	i32,
	i64,
	s
};

struct m_pool{
	void* chunk;
	size_t m_free;
	size_t allocated; 
};


void pool_free(void **ptr, size_t size, struct m_pool *pool);
void pool_destroy(struct m_pool *pool);
int pool_init(struct m_pool *pool);
int pool_alloc(struct m_pool *pool, void **ptr, size_t size, enum type t);



#endif /* mem_pool.h */
