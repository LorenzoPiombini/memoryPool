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

int pool_free(void **ptr, size_t size, struct m_pool *pool)
{
	if((*pool).allocated == 0) return -1;
	 
	if(!(*ptr)) return -1;

	if(!pool) return -1;
	
	if(size == 0) return -1;

	memset(*ptr,0,size);
	(*pool).allocated -= size;
	(*pool).m_free += size;

	if(!memory_blocks.fr_blocks) {
		if(pool_alloc(pool,(void **)&memory_blocks.fr_blocks,
			sizeof(struct free_blocks),1,internal) == -1) {
			fprintf(stderr,"can't alloc internal memory.\n");
			return -1;
		}

		memory_blocks.fr_blocks->block_start = *ptr;
		memory_blocks.fr_blocks->size = size;
		memory_blocks.fr_blocks->next = NULL;


	} else {
		struct free_blocks *temp = memory_blocks.fr_blocks->next;
		struct free_blocks *prev = memory_blocks.fr_blocks;

		while(temp){
			prev = prev->next;
			temp = temp->next;
		}

		if(pool_alloc(pool,(void **)&temp,
					sizeof(struct allocated_blocks),1,internal) == -1) {
			fprintf(stderr,"can't alloc internal memory.\n");
			return -1;
		}

		temp->block_start = *ptr;
		temp->size = size;
		temp->next = NULL;
		prev->next = temp;
	}

	*ptr = NULL;		
	return 0;
}

int pool_init(struct m_pool *pool)
{
	(*pool).chunk = (void*) malloc(CHUNK_SIZE);
	(*pool).internal_memory = (void*) malloc(INT_MEM_SIZE);
	if(!(*pool).chunk || !(*pool).internal_memory){
		fprintf(stderr,"%s() failed.\n",__func__);
		return -1;
	}

	memset(((*pool).chunk),0,CHUNK_SIZE);
	memset(((*pool).internal_memory),0,INT_MEM_SIZE);
	(*pool).allocated = 0;
	(*pool).m_free = CHUNK_SIZE;
	(*pool).allocated_internal = 0;
	(*pool).m_free_internal = CHUNK_SIZE;
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

			memory_blocks.al_blocks->block_start = *ptr;
			memory_blocks.al_blocks->size = (size * items);
			memory_blocks.al_blocks->next = NULL;

		}else {

			struct allocated_blocks *temp = memory_blocks.al_blocks->next;
			struct allocated_blocks *prev = memory_blocks.al_blocks;

			while(temp){
				prev = prev->next ; 
				temp = temp->next;
			}

			if(pool_alloc(pool,(void **)&temp,
						sizeof(struct allocated_blocks),1,internal) == -1) {
				fprintf(stderr,"can't alloc internal memory.\n");
				return -1;
			}

			temp->block_start = *ptr;
			temp->size = (size * items);
			temp->next = NULL;
			prev->next = temp;
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

			memory_blocks.al_blocks->block_start = *ptr;
			memory_blocks.al_blocks->size = (size * items);
			memory_blocks.al_blocks->next = NULL;

		}else {

			struct allocated_blocks *temp = memory_blocks.al_blocks->next;
			struct allocated_blocks *prev = memory_blocks.al_blocks;

			while(temp){
				temp = temp->next;
				prev = prev->next; 
			}

			if(pool_alloc(pool,(void **)&temp,
						sizeof(struct allocated_blocks),1,internal) == -1) {
				fprintf(stderr,"can't alloc internal memory.\n");
				return -1;
			}

			temp->block_start = *ptr;
			temp->size = (size * items);
			temp->next = NULL;
			prev->next = temp;
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
		}

		(*pool).allocated += (size * items);
		(*pool).m_free -= (size * items);

		if(!memory_blocks.al_blocks) {	
			if(pool_alloc(pool,(void **)&memory_blocks.al_blocks,
						sizeof(struct allocated_blocks),1,internal) == -1) {
				fprintf(stderr,"can't alloc internal memory.\n");
				return -1;
			}

			memory_blocks.al_blocks->block_start = *ptr;
			memory_blocks.al_blocks->size = (size * items);
			memory_blocks.al_blocks->next = NULL;

		}else {

			struct allocated_blocks *temp = memory_blocks.al_blocks->next;
			struct allocated_blocks *prev = memory_blocks.al_blocks;

			while(temp){
				temp = temp->next;
				prev = prev->next; 
			}

			if(pool_alloc(pool,(void **)&temp,
						sizeof(struct allocated_blocks),1,internal) == -1) {
				fprintf(stderr,"can't alloc internal memory.\n");
				return -1;
			}

			temp->block_start = *ptr;
			temp->size = (size * items);
			temp->next = NULL;
			prev->next = temp;
		}

		return 0;
	}
	case internal:
	{
		if(size > INT_MEM_SIZE || 
				(size * items) > INT_MEM_SIZE ||
				(size * items) > (*pool).m_free_internal) return -1;

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

	
	/*find the ptr in the allocated blocks*/
	struct allocated_blocks *temp = memory_blocks.al_blocks; 
	while(temp) {
		if(temp->block_start == *ptr)
			break;
	
		temp = temp->next;
	}

	if(!temp){
		fprintf(stderr,"pointer not found.\n");
		return -1;
	}

	size_t new_size = temp->size + (size * extend_items);

	/*look for big enough free blokcs*/
	if(memory_blocks.fr_blocks) {
		struct free_blocks *temp_fb = memory_blocks.fr_blocks;
		while(temp_fb){
			if(temp_fb->size > new_size)
				break;

			temp_fb = temp_fb->next;
		}
		
		/*
		 * check if the block is allign with the type t 
		 * */
		if(!is_align(temp_fb->block_start,size)) {
			uintptr_t addr =  align((uintptr_t)temp_fb->block_start, size);
			if(addr > (uintptr_t)((unsigned char*)(*pool).chunk + (CHUNK_SIZE - 1)))
				return -1;/*you should look for space along the chunk*/

			if(addr > (uintptr_t)((unsigned char*)temp_fb->block_start + (new_size - 1)) ||
				(addr+new_size) > (uintptr_t)((unsigned char*)temp_fb->block_start + (new_size - 1)))
				goto check_block_or_create_new_block;

		        /*TODO: create new free blocks*/		
			*ptr = (void*)addr;
		}

		memcpy(temp_fb->block_start,temp->block_start,temp->size);
		/*updte the pool allocation values*/
		(*pool).allocated += (new_size - temp->size);
		(*pool).m_free -= (new_size - temp->size);

		/*swap the blocks*/
		void* help = temp_fb->block_start;
		temp_fb->block_start = temp->block_start;
		temp->block_start = help;
		temp_fb->size = temp->size;
		temp->size = new_size;
		/*set the ointer to the new allocated block */
		*ptr = (unsigned char*)temp->block_start + temp_fb->size;
		return 0;
	}

	/*
	 * find a new block in the pool 
	 * - check if we can expand this block 
	 * */

	check_block_or_create_new_block:
	unsigned char *end = (unsigned char*)temp->block_start + new_size;
	unsigned char *start = (unsigned char*)temp->block_start;
	for(;start < end; start++)
		if(*start != 0)
			break;

	if(start == (end - 1) && *start == 0){
		/*we have enough space to expand this block */
		/* set the pointer to the first block of the new allocated block */
		*ptr = (unsigned char*)temp->block_start + temp->size;
		(*pool).allocated += (new_size - temp->size);
		(*pool).m_free -= (new_size - temp->size);
		temp->size = new_size;
		return 0;
	}

	/*here we have to find a new block */
	if(pool_alloc(pool,ptr,size,extend_items,t) == -1){
		fprintf(stderr,"can't realloc memory.\n");
		return -1;
	}

	/*copy existing block to new one*/
	memcpy(*ptr,temp->block_start,temp->size);

	if(pool_free(temp->block_start,temp->size,pool) == -1){
		fprintf(stderr,"can't free block starting at %p",temp->block_start);
		return -1;
	}

	temp->block_start = *ptr;
	*ptr = (unsigned char*)ptr + temp->size;
	temp->size = new_size;
	(*pool).allocated += new_size;
	(*pool).m_free -= new_size;
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


