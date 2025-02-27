#ifndef _MEM_POOL_H
#define _MEM_POOL_H

#include <stdalign.h>

#define CHUNK_SIZE 1048576 


enum type{
	i16,
	i32,
	i64,
	f32,
	f64,
	s,
	ud /*user defined data like struct */
};

struct m_pool{
	void* chunk;
	void* base_address;
	void* top_address;
	size_t m_free;
	size_t allocated;
};

struct free_block_list{
	void* block_start;
	size_t size;
	struct free_block_list *next;
};

extern struct free_block_list free_list;

void pool_free(void **ptr, size_t size, struct m_pool *pool);
void pool_destroy(struct m_pool *pool);
int pool_init(struct m_pool *pool);
int pool_alloc(struct m_pool *pool, void **ptr, size_t size,int items, enum type t);



#endif /* mem_pool.h */
