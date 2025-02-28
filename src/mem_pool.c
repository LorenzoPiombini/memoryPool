#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdalign.h>
#include "mem_pool.h"


/* meta deta from the memory pool */
struct Meta_data memory_blocks = {0};


static int is_align(void *ptr, size_t alignment);
static uintptr_t align(uintptr_t address, size_t alignment);

void pool_free(void **ptr, size_t size, struct m_pool *pool)
{
	if((*pool).allocated == 0) return;
	
	if(!(*ptr)) return;

	if(!pool) return;
	
	if(size == 0) return;



	memset(*ptr,0,size);
	(*pool).allocated -= size;
	(*pool).m_free += size;

	*ptr = NULL;		
}

int pool_init(struct m_pool *pool)
{
	(*pool).chunk = (void*) malloc(CHUNK_SIZE);
	(*pool).internal_memory = (void*) malloc(CHUNK_SIZE);
	if(!(*pool).chunk || !(*pool).internal_memory){
		fprintf(stderr,"%s() failed.\n",__func__);
		return -1;
	}

	memset(((*pool).chunk),0,CHUNK_SIZE);
	(*pool).allocated = 0;
	(*pool).m_free = CHUNK_SIZE;
	(*pool).base_address = (void*)((char*)(*pool).chunk);
	(*pool).top_address = (void*)((char*)(*pool).chunk + (CHUNK_SIZE - 1));

	return 0;
}

int pool_alloc(struct m_pool *pool, void **ptr, size_t size,int items, enum type t)
{
	if((!pool) || 
		( size > CHUNK_SIZE ) || 
		(((*pool).m_free) < size)) return -1;
	

	switch(t){
	case i32:
	{
		/* move along the chunk */
		/* this give you the first avaiable address */
		*ptr = (int*)((char*)(*pool).chunk + (*pool).allocated);
		
		if(!is_align(*ptr,sizeof(int))){
			uintptr_t addr =  align((uintptr_t)*ptr,(sizeof(int)));
			if(addr > (uintptr_t)((char*)(*pool).chunk + (CHUNK_SIZE - 1)))
				return -1;/*you should look for space along the chunk*/

			*ptr = (void*)addr;
		}

		(*pool).allocated += (size * items);
		(*pool).m_free -= (size * items);
		
		if(!memory_blocks.al_blocks) {	
			if(pool_alloc(pool,(void **)&memory_blocks.al_blocks,
						sizeof(struct allocated_blocks),1,internal) == -1) {
				fprintf(stderr,"can't alloc internal memory.\n");
				return -1;
			}

			(memory_blocks.al_blocks)->block_start = *ptr;
			(memory_blocks.al_blocks)->size = (size * items);
			(memory_blocks.al_blocks)->next = NULL;

		}else {

			struct allocated_blocks *temp = memory_blocks.al_blocks;
			if(!temp)
				memory_blocks.al_blocks->next = temp; 

			while(temp->next){
				memory_blocks.al_blocks->next = temp; 
				temp = temp->next;
			}

			if(pool_alloc(pool,(void **)&temp->next,
						sizeof(struct allocated_blocks),1,internal) == -1) {
				fprintf(stderr,"can't alloc internal memory.\n");
				return -1;
			}

			temp->next->block_start = *ptr;
			temp->next->size = (size * items);
			temp->next = NULL;
		}

		return 0;
	}
	case i64:
	{
		/* move along the chunk */
		*ptr = (long*)((char*)(*pool).chunk + (*pool).allocated);
		if(!is_align(*ptr,(sizeof(long)))){
			uintptr_t addr =  align((uintptr_t)*ptr,(sizeof(long)));
			if(addr > (uintptr_t)((char*)(*pool).chunk + (CHUNK_SIZE - 1)))
				return -1;/*you should look for space along the chunk*/

			*ptr = (void*)addr;
		}
		(*pool).allocated += (size * items);
		(*pool).m_free -= (size * items);
		return 0;
	}
	case s:
	{
		/*move along the chunk */

		*ptr = (char*)(*pool).chunk + (*pool).allocated;
		if(!is_align(*ptr,sizeof(char))){
			uintptr_t addr =  align((uintptr_t)*ptr,(sizeof(char)));
			if(addr > (uintptr_t)((char*)(*pool).chunk + (CHUNK_SIZE - 1)))
				return -1;/*you should look for space along the chunk*/
			
			*ptr = (void*)addr;
		}
		(*pool).allocated += (size * items);
		(*pool).m_free -= (size * items);

		if(!memory_blocks.al_blocks) {	
			if(pool_alloc(pool,(void **)&memory_blocks.al_blocks,
						sizeof(struct allocated_blocks),1,internal) == -1) {
				fprintf(stderr,"can't alloc internal memory.\n");
				return -1;
			}

			(memory_blocks.al_blocks)->block_start = *ptr;
			(memory_blocks.al_blocks)->size = (size * items);
			(memory_blocks.al_blocks)->next = NULL;

		}else {

			struct allocated_blocks *temp = memory_blocks.al_blocks;
			while(temp->next){
				temp = temp->next;
			}

			if(pool_alloc(pool,(void **)&temp->next,
						sizeof(struct allocated_blocks),1,internal) == -1) {
				fprintf(stderr,"can't alloc internal memory.\n");
				return -1;
			}

			temp->next->block_start = *ptr;
			temp->next->size = (size * items);
			temp->next = NULL;
		}

		return 0;
	}
	case ud:
	{
		*ptr = (void*)(char*)(*pool).chunk + (*pool).allocated;

		if(!is_align(*ptr,size)) {
			uintptr_t addr =  align((uintptr_t)*ptr, size);
			if(addr > (uintptr_t)((char*)(*pool).chunk + (CHUNK_SIZE - 1)))
				return -1;/*you should look for space along the chunk*/
			
			*ptr = (void*)addr;
		} else {
			*ptr = (char*)(*pool).chunk + (*pool).allocated;
		}

		(*pool).allocated += (size * items);
		(*pool).m_free -= (size * items);
		return 0;
	}
	case internal:
	{
		*ptr = (void*)(char*)(*pool).internal_memory + (*pool).allocated_internal;

		if(!is_align(*ptr,size)) {
			uintptr_t addr =  align((uintptr_t)*ptr, size);
			if(addr > (uintptr_t)((char*)(*pool).internal_memory + (INT_MEM_SIZE - 1)))
				return -1;/*you should look for space along the chunk*/
			
			*ptr = (void*)addr;
		} else {
			*ptr = (char*)(*pool).internal_memory + (*pool).allocated_internal;
		}

		(*pool).allocated_internal += (size * items);
		(*pool).m_free_internal -= (size * items);
		return 0;
	}
	default:
		fprintf(stderr,"type %d not supported.\n",t);
		return -1;
	}		
	
	return 0;	
}

int pool_realloc(struct m_pool *pool, void **ptr, size_t size, int extend_items, enum type t)
{
	
	if((!pool) || 
		( size > CHUNK_SIZE ) || 
		(((*pool).m_free) < size)) return -1;

	return 0;

}
void pool_destroy(struct m_pool *pool)
{	
	free((*pool).chunk);
	free((*pool).internal_memory);
	(*pool).m_free = 0;
	(*pool).allocated = 0;
	(*pool).m_free_internal = 0;
	(*pool).allocated_internal = 0;
}

static int is_align(void *ptr, size_t alignment)
{
	return ((uintptr_t)ptr % alignment) == 0;
}

static uintptr_t align(uintptr_t address, size_t alignment)
{
	return (uintptr_t)((address + (alignment - 1)) & ~(alignment - 1));
}


