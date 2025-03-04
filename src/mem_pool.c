#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdalign.h>
#include "mem_pool.h"


/* meta deta from the memory pool */
static struct Meta_data memory_blocks = {0};


static int is_align(void *ptr, size_t alignment);
static uintptr_t align(uintptr_t address, size_t alignment);
static int create_free_block(void** ptr, size_t size, struct m_pool *pool);
static void free_pool_internal(struct m_pool *pool, void **ptr, size_t size, enum type t);
static int destroy_block(void** ptr, struct m_pool *pool,enum type t);

int pool_free(void **ptr, size_t size, struct m_pool *pool)
{
	if((*pool).allocated == 0) return -1;
	 
	if(!(*ptr)) return -1;

	if(!pool) return -1;
	
	if(size == 0) return -1;

	memset(*ptr,0,size);
	(*pool).allocated -= size;
	(*pool).m_free += size;
	return 0;
}

int pool_init(struct m_pool *pool)
{
	(*pool).chunk = (void*) malloc(CHUNK_SIZE);
	(*pool).alloc_internal_memory = (void*) malloc(INT_MEM_SIZE);
	(*pool).free_internal_memory = (void*) malloc(INT_MEM_SIZE);
	if(!(*pool).chunk || !(*pool).alloc_internal_memory || !(*pool).free_internal_memory){
		fprintf(stderr,"%s() failed.\n",__func__);
		return -1;
	}

	memset((*pool).chunk,0,CHUNK_SIZE);
	memset((*pool).alloc_internal_memory,0,INT_MEM_SIZE);
	memset((*pool).free_internal_memory,0,INT_MEM_SIZE);

	(*pool).allocated = 0;
	(*pool).m_free = CHUNK_SIZE;
	(*pool).allocated_internal = 0;
	(*pool).m_free_internal = 0;

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
						sizeof(struct allocated_blocks),1,inter_alloc) == -1) {
				fprintf(stderr,"can't alloc internal memory.\n");
				return -1;
			}

			memory_blocks.al_blocks->block_start = *ptr;
			memory_blocks.al_blocks->size = size * items;
			memory_blocks.al_blocks->next = NULL;

		}else {

			struct allocated_blocks *temp = memory_blocks.al_blocks->next;
			struct allocated_blocks *prev = memory_blocks.al_blocks;

			while(temp){
				prev = prev->next ; 
				temp = temp->next;
			}

			if(pool_alloc(pool,(void **)&temp,
						sizeof(struct allocated_blocks),1,inter_alloc) == -1) {
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
						sizeof(struct allocated_blocks),1,inter_alloc) == -1) {
				fprintf(stderr,"can't alloc internal memory.\n");
				return -1;
			}

			memory_blocks.al_blocks->block_start = *ptr;
			memory_blocks.al_blocks->size = size * items;
			memory_blocks.al_blocks->next = NULL;

		}else {
			struct allocated_blocks *temp = memory_blocks.al_blocks->next;
			struct allocated_blocks *prev = memory_blocks.al_blocks;

			while(temp){
				temp = temp->next;
				prev = prev->next; 
			}

			if(pool_alloc(pool,(void **)&temp,
						sizeof(struct allocated_blocks),1,inter_alloc) == -1) {
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
						sizeof(struct allocated_blocks),1,inter_alloc) == -1) {
				fprintf(stderr,"can't alloc internal memory.\n");
				return -1;
			}

			memory_blocks.al_blocks->block_start = *ptr;
			memory_blocks.al_blocks->size = (size * items);
			memory_blocks.al_blocks->next = NULL;

		}else {

			struct allocated_blocks *temp = memory_blocks.al_blocks->next;
			struct allocated_blocks *prev = memory_blocks.al_blocks;

			int counter = 0;
			while(temp){
				printf("iter nr %d.\n",++counter);
				temp = temp->next;
				prev = prev->next; 
			}

			if(pool_alloc(pool,(void **)&temp,
						sizeof(struct allocated_blocks),1,inter_alloc) == -1) {
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
	case inter_alloc:
	{
		if(size > INT_MEM_SIZE || 
				(size * items) > INT_MEM_SIZE ||
				(size * items) > (INT_MEM_SIZE - (*pool).allocated_internal)) return -1;

		*ptr = (void*)((char*)(*pool).alloc_internal_memory + (*pool).allocated_internal);
		if(!is_align(*ptr,size)) {
			uintptr_t addr =  align((uintptr_t)*ptr, size);
			if(addr > (uintptr_t)((char*)(*pool).alloc_internal_memory + (INT_MEM_SIZE - 1)))
				return -1;/*you should look for space along the chunk*/
			
			*ptr = (void*)addr;
		}

		(*pool).allocated_internal += (size * items);
		return 0;
	}
	case inter_free:
	{
		if(size > INT_MEM_SIZE || 
				(size * items) > INT_MEM_SIZE ||
				(size * items) > (INT_MEM_SIZE - (*pool).m_free_internal)) return -1;

		*ptr = (void*)((char*)(*pool).free_internal_memory + (*pool).m_free_internal);
		if(!is_align(*ptr,size)) {
			uintptr_t addr =  align((uintptr_t)*ptr, size);
			if(addr > (uintptr_t)((char*)(*pool).free_internal_memory + (INT_MEM_SIZE - 1)))
				return -1;/*you should look for space along the chunk*/
			
			*ptr = (void*)addr;
		}

		(*pool).m_free_internal += (size * items);
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

	/*look for a big enough free blokcs*/
	if(memory_blocks.fr_blocks) {
		struct free_blocks *temp_fb = memory_blocks.fr_blocks;
		while(temp_fb){
			if(temp_fb->size > new_size)
				break;

			temp_fb = temp_fb->next;
		}

		if(!temp_fb){
			goto check_block_or_create_new_block;
		
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

			*ptr = (void*)addr;

		        /* 
			 * here we create new free blocks
			 * based on the alignment 
			 * if the aliginment finds an address within the free block
			 * we need to create two new free blocks
			 * */	

			if(temp_fb->block_start < *ptr) {
				ptrdiff_t free_block_size = ((unsigned char*)*ptr - (unsigned char*)temp_fb->block_start);
				if(((unsigned char*)temp_fb->block_start + (temp_fb->size - 1)) > ( (unsigned char*)*ptr + (new_size - 1) )) {
					ptrdiff_t second_free_block_size =  ((unsigned char*)temp_fb->block_start + (temp_fb->size - 1)) - ((unsigned char*)*ptr + (new_size -1 ));
					if(create_free_block((void**)((unsigned char*)*ptr + new_size), (size_t) second_free_block_size,pool) == -1) {
						fprintf(stderr,"can't create free blocks metadata.\n");
						return -1;
					}
				}
				
				temp_fb->size = free_block_size; 
				
				/*copy the memory to the new found block*/
				memcpy(*ptr,temp->block_start,temp->size);

				/*updte the pool allocation values*/
				(*pool).allocated += (new_size - temp->size);
				(*pool).m_free -= (new_size - temp->size);

				/*the originally allocated block becomes free so we create a free block from it*/
				if(create_free_block(&temp->block_start,temp->size,pool) == -1) {
					fprintf(stderr,"can't create free blocks metadata.\n");
					return -1;
				}

				/*update the allocated block with the new address*/
				temp->block_start = *ptr;

				/*set the pointer to the first address of the reallocated memory*/
				*ptr = (unsigned char*)*ptr + temp->size;
				/*update the size of the new block */
				temp->size = new_size;

			} else if(temp_fb->block_start == *ptr) {
				if(((unsigned char*)*ptr + new_size) < ((unsigned char*)temp_fb->block_start + (temp_fb->size - 1))){
					temp_fb->block_start = (void*)(unsigned char*)*ptr + new_size;
					temp_fb->size = temp_fb->size - new_size;

					/*copy the memory to the new found block*/
					memcpy(*ptr,temp->block_start,temp->size);

					/*updte the pool allocation values*/
					(*pool).allocated += (new_size - temp->size);
					(*pool).m_free -= (new_size - temp->size);

					/*the originally allocated block becomes free so we create a free block from it*/
					if(create_free_block(&temp->block_start,temp->size,pool) == -1) {
						fprintf(stderr,"can't create free blocks metadata.\n");
						return -1;
					}

					/*update the allocated block with the new address*/
					temp->block_start = *ptr;

					/*set the pointer to the first address of the reallocated memory*/
					*ptr = (unsigned char*)*ptr + temp->size;
					/*update the size of the new block */
					temp->size = new_size;
					
					return 0;
				}
			}			
		}

		/*alignment is good */
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
		/*set the pointer to the new allocated block */
		*ptr = (unsigned char*)temp->block_start + temp_fb->size;
		return 0;
	}

	/*
	 * - check if we can expand this block 
	 * or find a new block in the pool 
	 * */

	check_block_or_create_new_block:
	unsigned char *end = (unsigned char*)temp->block_start + (new_size - 1);
	unsigned char *start = (unsigned char*)temp->block_start + temp->size;
	for(;start < end; start++){
		if(*start != 0)
			break;
	}

	if(start == end  && *start == 0){
		/*we have enough space to expand this block */
		/* set the pointer to the first block of the new allocated block */
		(*pool).allocated += (new_size - temp->size);
		(*pool).m_free -= (new_size - temp->size);
		temp->size = new_size;
		return 0;
	}

	/*here we have to find a new block */
	if(pool_alloc(pool,ptr,size,new_size/size,t) == -1){
		fprintf(stderr,"can't realloc memory.\n");
		return -1;
	}

	/*copy existing block to new one*/
	memcpy(*ptr,temp->block_start,temp->size);

	if(pool_free(&temp->block_start,temp->size,pool) == -1){
		fprintf(stderr,"can't free block starting at %p",temp->block_start);
		return -1;
	}

	if(create_free_block(&temp->block_start, temp->size, pool) == -1){
		fprintf(stderr,"can't create free block starting at %p",temp->block_start);
		return 1;
	}
	return 0;
}

void pool_destroy(struct m_pool *pool)
{	
	free((*pool).chunk);
	free((*pool).alloc_internal_memory);
	free((*pool).free_internal_memory);

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


static int create_free_block(void** ptr, size_t size, struct m_pool *pool)
{

	if(!memory_blocks.fr_blocks) {
		if(pool_alloc(pool,(void **)&memory_blocks.fr_blocks,
			sizeof(struct free_blocks),1,inter_free) == -1) {
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
			temp = temp->next;
			prev = prev->next;
		}

		if(pool_alloc(pool,(void **)&temp,
					sizeof(struct allocated_blocks),1,inter_free) == -1) {
			fprintf(stderr,"can't alloc internal memory.\n");
			return -1;
		}

		temp->block_start = *ptr;
		temp->size = size;
		temp->next = NULL;
		prev->next = temp;
	}

	if(destroy_block(ptr,pool,inter_alloc) == -1){
		fprintf(stderr,"block not found");
		return -1;
	}

	return 0;

}

static void free_pool_internal(struct m_pool *pool, void **ptr, size_t size, enum type t)
{
	memset(*ptr,0,size);
	switch(t){
	case inter_alloc:
		(*pool).allocated_internal -= size;
		break;
	case inter_free:	
		(*pool).m_free_internal -= size;
		break;
	default:
		break;
	}
}

static int destroy_block(void** ptr, struct m_pool *pool,enum type t)
{
	switch(t) {
	case inter_alloc:
	{
		struct allocated_blocks *temp = memory_blocks.al_blocks;
		if(!temp) return 0;


		if(temp->block_start == *ptr) {
			memory_blocks.al_blocks = temp->next;
			temp->next = NULL;
			free_pool_internal(pool,(void**)&temp,sizeof(struct allocated_blocks),t);
			return 0;

		}

		temp = temp->next;
		struct allocated_blocks *prev = memory_blocks.al_blocks;
		while(temp) {

			if(temp->block_start == *ptr)
				break;

			temp = temp->next;
			prev= prev->next;
		}

		if(!temp) return 0;

		prev->next = prev->next->next;
		temp->next =NULL;
		free_pool_internal(pool,(void**)&temp,sizeof(struct allocated_blocks),t);
		return 0;
	}
	case inter_free:
	{
		struct free_blocks *temp = memory_blocks.fr_blocks;
		if(!temp) return 0;


		if(temp->block_start == *ptr) {
			memory_blocks.fr_blocks = temp->next;
			temp->next = NULL;
			free_pool_internal(pool,(void**)&temp,sizeof(struct free_blocks),t);
			return 0;

		}

		temp = temp->next;
		struct free_blocks *prev = memory_blocks.fr_blocks;
		while(temp) {

			if(temp->block_start == *ptr)
				break;

			temp = temp->next;
			prev= prev->next;
		}

		if(!temp) return 0;

		prev->next = prev->next->next;
		temp->next =NULL;
		free_pool_internal(pool,(void**)&temp,sizeof(struct free_blocks),t);
		return 0;
	}
	default:
		return 0;
	}
	
}
