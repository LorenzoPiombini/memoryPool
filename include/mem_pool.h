#ifndef _MEM_POOL_H
#define _MEM_POOL_H


#define CHUNK_SIZE 1048576  /* 1 Mb */
#define INT_MEM_SIZE 1024*500 /*500kb*/


enum type{
	i16,
	i32,
	i64,
	f32,
	f64,
	s,
	ud,/*user defined data like struct */
	internal	
};

struct m_pool{
	void* chunk;
	void* internal_memory;
	void* base_address;
	void* top_address;
	size_t m_free;
	size_t allocated;
	/*internal mem data for META DATA*/
	size_t allocated_internal;
	size_t m_free_internal;
};


struct free_blocks{
	void* block_start;
	size_t size;
	struct free_blocks *next;
};


struct allocated_blocks{
	void *block_start;
	size_t size;
	struct allocated_blocks *next;
};

struct Meta_data{
	struct allocated_blocks *al_blocks;
	struct free_blocks *fr_blocks;
};

extern struct Meta_data memory_blocks;


int pool_free(void **ptr, size_t size, struct m_pool *pool);
void pool_destroy(struct m_pool *pool);
int pool_init(struct m_pool *pool);
int pool_alloc(struct m_pool *pool, void **ptr, size_t size,int items, enum type t);
int pool_realloc(struct m_pool *pool, void **ptr, size_t size, int extend_items, enum type t);




#endif /* mem_pool.h */
